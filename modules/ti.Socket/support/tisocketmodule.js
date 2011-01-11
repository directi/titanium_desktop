function replaceMethod(obj, methodName, newMethod)
{
	var originalMethodName = methodName + "_orig";
	obj[originalMethodName] = obj[methodName];
	var fn = function()
	{
		newMethod.apply(window, arguments);
		obj[originalMethodName].apply(obj, arguments);
	};
	obj[methodName] = fn;
}

replaceMethod(Titanium.Socket.createTCPSocket,  function() { 
	var a = Titanium.Socket.createTCPSocket_orig();
	a.onReadComplete = function(arg) { onClose(arg) }
	return a;
});
