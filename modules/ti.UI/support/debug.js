window.pauseDebugger = function() 
	{
		if(typeof(Titanium._debugTimerId) != 'undefined' && Titanium._debugTimerId != null) {
			Titanium.clearTimeout(Titanium._debugTimerId);
			Titanium._debugTimerId = null;
		}
		Titanium.debuggerPaused();
	}

window.resumeDebugger = function(stepping)
	{
		if(stepping)
		{
			Titanium._debugTimerId = Titanium.setTimeout(function() { 
				Titanium.debuggerResumed();
				Titanium._debugTimerId = null; 
			}, 1000);
		} else {
			if(typeof(Titanium._debugTimerId) != 'undefined' && Titanium._debugTimerId != null) {
				Titanium.clearTimeout(Titanium._debugTimerId);
				Titanium._debugTimerId = null;
			}
			Titanium.debuggerResumed();
		}
	}