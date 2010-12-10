window._debugTimerId = 0;

window.pauseDebugger = function() 
	{
		if(_debugTimerId != 0)
			Titanium.clearTimeout(_debugTimerId);
		Titanium.debuggerPaused();
	}

window.resumeDebugger = function(stepping)
	{
		if(stepping)
		{
			_debugTimerId = Titanium.setTimeout(function() { Titanium.debuggerResumed(); _debugTimerId = 0 }, 1000);
		} else {
			Titanium.debuggerResumed();
		}
	}