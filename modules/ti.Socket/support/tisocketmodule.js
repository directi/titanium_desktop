(function(){
function replaceMethod(obj, methodName, newMethod)
{
	var originalMethodName = methodName + "_orig";
	obj[originalMethodName] = obj[methodName];
	var fn = function(){
		return newMethod.apply(obj, arguments);
	};
	obj[methodName] = fn;
}

replaceMethod(Titanium.Socket, "createTCPSocket",  function(server, port) { 
	var a = Titanium.Socket.createTCPSocket_orig(server, port);
	a.onReadComplete = function(arg) { a.onClose(arg); }
	a.connectNB_orig = a.connectNB;
	a.connectNB = function(){
		a.connectNB_orig();
		return true;
	}
	return a;
});

Titanium.Network = Titanium.Socket;
}());