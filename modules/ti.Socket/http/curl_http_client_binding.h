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
	public:
		CURLHTTPClientBinding(Host* host);
		virtual ~CURLHTTPClientBinding();

		enum HTTP_STATE_en
		{
			HTTP_UNSENT = 0,
			HTTP_OPENED,
			HTTP_HEADERS_RECEIVED,
			HTTP_LOADING,
			HTTP_DONE
		}readyState;

		void ChangeState(HTTP_STATE_en _readyState);

		static CURLMULTIClient multiClient;

	private:
		Host* host;

		std::string httpMethod;
		std::string url;
		std::string filename;
		int timeout;
		KMethodRef onHeaderReceived;
		KMethodRef onDataChunkReceived;
		KMethodRef onHTTPDone;
		long maxRedirects;

		std::vector<std::string> requestHeaders;
		std::map<std::string, std::string> requestCookies;
		CURLEASYClient * easy;

		void ExecuteRequest(const std::string & data);

		void SetOnHeaderReceived(const ValueList& args, KValueRef result);
		void SetOnDataChunkReceived(const ValueList& args, KValueRef result);
		void SetOnHTTPDone(const ValueList& args, KValueRef result);

		void Open(const ValueList& args, KValueRef result);
		void getReadyState(const ValueList& args, KValueRef result);
		void getHTTPStatus(const ValueList& args, KValueRef result);
		void Send(const ValueList& args, KValueRef result);
		void SetRequestHeader(const ValueList& args, KValueRef result);
		void SetTimeout(const ValueList& args, KValueRef result);
		void SetCookie(const ValueList& args, KValueRef result);
		void ClearCookies(const ValueList& args, KValueRef result);
		void GetTimeout(const ValueList& args, KValueRef result);
		void GetMaxRedirects(const ValueList& args, KValueRef result);
		void SetMaxRedirects(const ValueList& args, KValueRef result);
		void GetResponseHeader(const ValueList& args, KValueRef result);
		void GetResponseHeaders(const ValueList& args, KValueRef result);
		void Abort(const ValueList& args, KValueRef result);

		//void Receive(const ValueList& args, KValueRef result);
		//void GetCookie(const ValueList& args, KValueRef result);
	};
}

#endif
