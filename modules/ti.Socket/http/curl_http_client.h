/**
 * @author Mital Vora <mital.d.vora@gmail.com>
 */

#ifndef _CURL_HTTP_CLIENT_H_
#define _CURL_HTTP_CLIENT_H_

#include <kroll/kroll.h>
#include <kroll/utils/Thread.h>
#include <curl/curl.h>
#define SET_CURL_OPTION(handle, option, value) \
	{\
		CURLcode result = curl_easy_setopt(handle, option, value); \
		if (CURLE_OK != result) \
		{ \
			GetLogger()->Error("Failed to set cURL handle option ("#option"): %s", \
				curl_easy_strerror(result)); \
		} \
	}

namespace ti
{
	class CURLHTTPClientBinding;
	
	class CURLEASYClient
	{
	private:
		CURL *curl_handle;
		std::string url;
		bool sawHTTPStatus;
		long httpStatus;
		bool aborted;
		std::string statusText;
		CURLHTTPClientBinding * binding;
		KMethodRef onHeaderReceived;
		KMethodRef onDataChunkReceived;
		std::map<std::string, std::string> responseHeaders;
		std::map<std::string, std::string> nextResponseHeaders;
		std::vector<std::string> responseCookies;

		void ParseHTTPStatus(const std::string& header);
		void getResponseCookie(const std::string &cookieLine);

	public:
		CURLEASYClient(const std::string & url, CURLHTTPClientBinding * _binding);
		~CURLEASYClient();
		CURL *getCURLHandle() const { return curl_handle; }
		bool getHTTPStatus() const { return httpStatus; }
		std::string getResponseHeader(const std::string &name) const;
		void getResponseHeaders(std::map<std::string, std::string> &responseHeaders) const;
		void getResponseCookies(std::vector<std::string> &responseCookies) const;
		bool isAborted() const { return this->aborted; }
		void abort() { this->aborted = true; }

		void setOnHeaderReceived(KMethodRef _onHeaderReceived);
		void setOnDataChunkReceived(KMethodRef _onDataChunkReceived);
		void setProxyUsernamePassword(const std::string & userpwd);
		void setRequestHeaders(const std::vector<std::string> & requestHeaders);
		void setRequestCookies(const std::map<std::string, std::string> & cookies);
		void setTimeout(int timeout);
		void setMaxRedirects(long maxRedirects);
		void gotHeader(const std::string& header);
		void gotData(char* buffer, size_t numberOfBytes);
		void done();
	};

	class CURLMULTIClient
		: public kroll::Runnable
	{
		CURLM *multi_handle;
		bool started;
		kroll::Thread* thread;
		std::list<CURLEASYClient*> requests;
		std::list<CURLEASYClient*> requestsBeingExecuted;

		void addEasyClients();
		void removeCompletedJobs();
		void remove(CURLEASYClient * easy);

	public:
		CURLMULTIClient();
		virtual ~CURLMULTIClient();
		void add(CURLEASYClient * easy);
		void perform();

		virtual void run();
	};
}
#endif