/**
 * @author Mital Vora <mital.d.vora@gmail.com>
 */

#ifndef _CURL_HTTP_CLIENT_BINDING_H_
#define _CURL_HTTP_CLIENT_BINDING_H_

#include "curl_http_client.h"


namespace ti
{
	class CURLHTTPClientBinding : public KEventObject
	{
	private:

		Host* host;
		enum HTTP_STATE_en
		{
			HTTP_UNSENT = 0,
			HTTP_OPENED,
			HTTP_HEADERS_RECEIVED,
			HTTP_LOADING,
			HTTP_DONE
		}readyState;

		std::string httpMethod;
		std::string url;
		std::string filename;
		int timeout;
		KMethodRef onHeaderReceived;
		KMethodRef onDataChunkReceived;
		std::vector<std::string> requestHeaders;
		long maxRedirects;
		CURLEASYClient * easy;

		void ChangeState(HTTP_STATE_en _readyState);
		void ExecuteRequest(const std::string & data);

		void Open(const ValueList& args, KValueRef result);
		void SetOnHeaderReceived(const ValueList& args, KValueRef result);
		void SetOnDataChunkReceived(const ValueList& args, KValueRef result);
		void saveToFile(const ValueList& args, KValueRef result);
		void getReadyState(const ValueList& args, KValueRef result);
		void getHTTPStatus(const ValueList& args, KValueRef result);
		void Send(const ValueList& args, KValueRef result);
		void SetRequestHeader(const ValueList& args, KValueRef result);
		void SetTimeout(const ValueList& args, KValueRef result);
		void GetTimeout(const ValueList& args, KValueRef result);
		void GetMaxRedirects(const ValueList& args, KValueRef result);
		void SetMaxRedirects(const ValueList& args, KValueRef result);
		void GetResponseHeader(const ValueList& args, KValueRef result);
		void GetResponseHeaders(const ValueList& args, KValueRef result);
		
		//void Abort(const ValueList& args, KValueRef result);
		//void Receive(const ValueList& args, KValueRef result);
		//void SetCookie(const ValueList& args, KValueRef result);
		//void ClearCookies(const ValueList& args, KValueRef result);
		//void GetCookie(const ValueList& args, KValueRef result);

	public:
		CURLHTTPClientBinding(Host* host);
		virtual ~CURLHTTPClientBinding();

		static CURLMULTIClient multiClient;
	};
}

#endif
