/**
 * @author Mital Vora <mital.d.vora@gmail.com>
 */

#include "curl_http_client.h"
#include "curl_http_client_binding.h"

#include <boost/algorithm/string.hpp>

namespace ti
{
	static Logger* GetLogger()
	{
		static Logger* logger = Logger::Get("Socket.CURLEASYClient");
		return logger;
	}

	static size_t CurlHeaderCallback(void* ptr, size_t size, size_t nmemb,
		CURLEASYClient* client)
	{
		size_t headerLineSize = size * nmemb;
		std::string header(static_cast<char*>(ptr), headerLineSize);
		client->gotHeader(header);
		return headerLineSize;
	}
	static size_t CurlWriteCallback(void* buffer, size_t size, size_t nmemb,
		CURLEASYClient* client)
	{
		size_t dataLength = size * nmemb;
		client->gotData(static_cast<char*>(buffer), dataLength);
		return dataLength;
	}

	int CurlProgressCallback(CURLEASYClient* client, double dltotal, double dlnow, double ultotal, double ulnow)
	{
		//if (client->IsAborted())
		//	return CURLE_ABORTED_BY_CALLBACK;
		//else
			return 0;

		//client->RequestDataSent(ulnow, ultotal);
	}

	CURLEASYClient::CURLEASYClient(const std::string & url, CURLHTTPClientBinding * _binding)
		: curl_handle(curl_easy_init()),
		url(url),
		sawHTTPStatus(false),
		httpStatus(0),
		binding(_binding)
	{
		SET_CURL_OPTION(curl_handle, CURLOPT_URL, url.c_str());
		SET_CURL_OPTION(curl_handle, CURLOPT_PRIVATE, (void *) this);
		SET_CURL_OPTION(curl_handle, CURLOPT_HEADERFUNCTION, &CurlHeaderCallback);
		SET_CURL_OPTION(curl_handle, CURLOPT_WRITEFUNCTION, &CurlWriteCallback);
		SET_CURL_OPTION(curl_handle, CURLOPT_PROGRESSFUNCTION, &CurlProgressCallback);
		SET_CURL_OPTION(curl_handle, CURLOPT_WRITEHEADER, this);
		SET_CURL_OPTION(curl_handle, CURLOPT_WRITEDATA, this);
		SET_CURL_OPTION(curl_handle, CURLOPT_PROGRESSDATA, this);

		// non negative number means don't verify peer cert - we might want to 
		// make this configurable in the future
		SET_CURL_OPTION(curl_handle, CURLOPT_SSL_VERIFYPEER, 1);

		// Progress must be turned on for CURLOPT_PROGRESSFUNCTION to be called.
		SET_CURL_OPTION(curl_handle, CURLOPT_NOPROGRESS, 0);

	}

	CURLEASYClient::~CURLEASYClient()
	{
		done();
		curl_easy_cleanup(curl_handle);
	}

	void CURLEASYClient::setOnHeaderReceived(KMethodRef _onHeaderReceived)
	{
		this->onHeaderReceived = _onHeaderReceived;
	}
	void CURLEASYClient::setOnDataChunkReceived(KMethodRef _onDataChunkReceived)
	{
		this->onDataChunkReceived = _onDataChunkReceived;
	}

	void CURLEASYClient::setProxyUsernamePassword(const std::string & userpwd)
	{
		SET_CURL_OPTION(curl_handle, CURLOPT_PROXYUSERPWD, userpwd.c_str());
	}

	void CURLEASYClient::setRequestHeaders(const std::vector<std::string> & requestHeaders)
	{
		if (!requestHeaders.empty())
		{
			struct curl_slist* curlHeaders = NULL;
			for (size_t i = 0; i < requestHeaders.size(); i++)
			{
				curlHeaders = curl_slist_append(curlHeaders, requestHeaders[i].c_str());
			}

			SET_CURL_OPTION(curl_handle, CURLOPT_HTTPHEADER, curlHeaders);
		}
	}

	void CURLEASYClient::setTimeout(int timeout)
	{
		if (timeout > 0)
		{
			SET_CURL_OPTION(curl_handle, CURLOPT_TIMEOUT_MS, timeout);
			SET_CURL_OPTION(curl_handle, CURLOPT_DNS_CACHE_TIMEOUT, timeout/1000);
		}
	}
	void CURLEASYClient::setMaxRedirects(long maxRedirects)
	{
		SET_CURL_OPTION(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
		SET_CURL_OPTION(curl_handle, CURLOPT_MAXREDIRS, maxRedirects);
	}

	bool isRedirect(int httpStatus)
	{
		if ((httpStatus / 100) == 3)
		{
			return true;
		}
		return false;
	}

	void CURLEASYClient::gotHeader(const std::string& header)
	{
		// We want to keep all the headers here and only set them on the client
		// in chunks. The reason for this is that this is that cURL uses this
		// callback for all headers even those used in authentication negotiation
		// and redirects. We only want the last chunk of headers to be on the client
		// at the end of the request.
		if (header == "\r\n" || header == "\n") // This is the end of header chunk
		{
			this->sawHTTPStatus = false;

			this->responseHeaders.clear();
			for(std::map<std::string, std::string>::const_iterator
				i = this->nextResponseHeaders.begin();
				i != this->nextResponseHeaders.end();
			i++)
			{
				this->responseHeaders[i->first] = i->second;
			}
			this->nextResponseHeaders.clear();

			httpStatus = 0;
			curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &httpStatus);

			const char* effectiveURL;
			curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &effectiveURL);

			if (!isRedirect(httpStatus))
			{
				binding->ChangeState(CURLHTTPClientBinding::HTTP_HEADERS_RECEIVED);
				binding->ChangeState(CURLHTTPClientBinding::HTTP_LOADING);
				if (!this->onHeaderReceived.isNull())
				{
					RunOnMainThread(this->onHeaderReceived);
				}
			}

			// Update the URL in the case that this is redirect
			this->url = effectiveURL;
		}
		else // Normal header
		{
			if (!sawHTTPStatus && header.find("HTTP") == 0)
			{
				this->ParseHTTPStatus(header);
				sawHTTPStatus = true;
				return;
			}

			size_t splitPos = header.find(":");
			if (splitPos == std::string::npos)
				return;

			std::string headerName(header.substr(0, splitPos));
			std::string headerValue(FileUtils::Trim(header.substr(splitPos + 1)));
			nextResponseHeaders[headerName] = headerValue;

			boost::to_lower(headerName);
			// TODO:
			//if (headerName == "set-cookie")
			//	this->GetResponseCookie(headerValue);
		}
	}

	void CURLEASYClient::ParseHTTPStatus(const std::string& header)
	{
		size_t numberOfSpaces = 0;
		size_t position = 0;
		while (numberOfSpaces < 2 && position < header.size())
		{
			if (header[position] == ' ')
				numberOfSpaces++;
			position++;
		}

		if (position < header.size())
		{	
			this->statusText = FileUtils::Trim(header.substr(position)).c_str();
		}
	}

	void CURLEASYClient::gotData(char* buffer, size_t bufferSize)
	{
		if (!onDataChunkReceived.isNull())
		{
			RunReadJobOnMainThread(this->onDataChunkReceived, buffer, bufferSize);
		}
		//this->FireEvent(Event::HTTP_DATA_RECEIVED);
	}

	void CURLEASYClient::done()
	{
		if (binding)
		{
			binding->ChangeState(CURLHTTPClientBinding::HTTP_DONE);
		}
	}

	std::string CURLEASYClient::getResponseHeader(const std::string &name) const
	{
		std::string header_value;
		std::map<std::string, std::string>::const_iterator
			oIter = this->responseHeaders.find(name);
		if (oIter != this->responseHeaders.end())
		{
			header_value = oIter->second;
		}
		return header_value;
	}

	void CURLEASYClient::getResponseHeaders(std::map<std::string, std::string> &responseHeaders) const
	{
		for (std::map<std::string, std::string>::const_iterator
			oIter = this->responseHeaders.begin();
			oIter != this->responseHeaders.end();
		oIter++)
		{
			responseHeaders[oIter->first] = oIter->second;
		}
	}

	CURLMULTIClient::CURLMULTIClient()
		: multi_handle(curl_multi_init()),
		started(false),
		thread(new kroll::Thread())
	{
	}

	CURLMULTIClient::~CURLMULTIClient()
	{
		curl_multi_cleanup(multi_handle);
		delete thread;
		thread = NULL;
	}

	void CURLMULTIClient::add(CURLEASYClient * easy)
	{
		size_t size = requests.size();
		requests.push_back(easy);
		if(!started)
		{
			started = true;
			thread->start(this);
		}
	}
	void CURLMULTIClient::remove(CURLEASYClient * easy)
	{
		curl_multi_remove_handle(multi_handle, easy->getCURLHandle());
		for(std::list<CURLEASYClient*>::iterator
			oIter = requestsBeingExecuted.begin();
			oIter != requestsBeingExecuted.end();
		oIter++)
		{
			if(*oIter == easy)
			{
				(*oIter)->done();
				requestsBeingExecuted.erase(oIter);
				break;
			}
		}
	}

	void CURLMULTIClient::run()
	{

		while(requests.size() + requestsBeingExecuted.size()  > 0)
		{
			this->perform();
		}
		started = false;
	}


	void CURLMULTIClient::addEasyClients()
	{
		while(requests.size())
		{
			CURLEASYClient * easy = requests.front();
			curl_multi_add_handle(multi_handle, easy->getCURLHandle());
			requests.pop_back();
			requestsBeingExecuted.push_back(easy);
			// store easy with another list of currently serving requests.
		}
	}

	void CURLMULTIClient::removeCompletedJobs()
	{
		// check the curl messages indicating completed transfers
		// and free their resources
		while (true)
		{
			int messagesInQueue;
			CURLMsg* msg = curl_multi_info_read(multi_handle, &messagesInQueue);
			if (!msg)
				break;

			// find the node which has same d->m_handle as completed transfer
			CURL* handle = msg->easy_handle;
			if(!handle) { continue; }
			CURLEASYClient* job = 0;
			CURLcode err = curl_easy_getinfo(handle, CURLINFO_PRIVATE, &job);
			if (CURLE_OK != err) { continue; }
			if(!job) { continue; }
			if(job->getCURLHandle() != handle) { continue; }

			if (job->isCancelled())
			{
				remove(job);
				continue;
			}

			if (CURLMSG_DONE != msg->msg)
				continue;

			if (CURLE_OK != msg->data.result)
			{
				char* url = 0;
				curl_easy_getinfo(handle, CURLINFO_EFFECTIVE_URL, &url);
				std::cerr << "Curl ERROR for url='" << url 
					<< "', error: '" << curl_easy_strerror(msg->data.result) 
					<< "'" << std::endl;
			}
			remove(job);
		}

	}

	void CURLMULTIClient::perform()
	{
		this->addEasyClients();

		fd_set fdread;
		fd_set fdwrite;
		fd_set fdexcep;
		int maxfd = 0;

		/* set a suitable timeout to play around with */ 
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 5000; // 5 miliseconds

		// Retry 'select' if it was interrupted by a process signal.
		int rc = 0;
		do
		{
			FD_ZERO(&fdread);
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdexcep);
			curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
			// When the 3 file descriptors are empty, winsock will return -1
			// and bail out, stopping the file download. So make sure we
			// have valid file descriptors before calling select.
			if (maxfd >= 0)
				rc = ::select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
		} while (rc == -1);

		if(-1 == rc) { return; }

		int runningHandles = 0;
		CURLMcode ret;
		while ((ret = curl_multi_perform(multi_handle, &runningHandles)) == CURLM_CALL_MULTI_PERFORM) { }

		this->removeCompletedJobs();
	}

}

