function onDetectionButtonAction() {
	console.log('onDetctionButtonAction()');
	document.getElementsByClassName('blink')[0].style.display = 'block';
}

function offDetectionButtonAction() {
	console.log('offDetctionButtonAction()');
	document.getElementsByClassName('blink')[0].style.display = 'none';
}

const blobToBase64 = blob => {
	const reader = new FileReader();
	reader.readAsDataURL(blob);
	return new Promise(resolve => {
	  reader.onloadend = () => {
		resolve(reader.result);
	  };
	});
};

function getLiveFrame() {
	const userAction = async () => {
		const response = await fetch('http://0.0.0.0:8081/liveframe', {
			method: "GET", mode: 'cors',
		});
		const imgBlob = await response.blob();

		let reader = new FileReader();
		reader.readAsDataURL(imgBlob);
		reader.onloadend = function() {
			var base64data = reader.result;
			guiUpdateLiveStream(base64data);
			console.log(base64data);
		}
	
	}
	userAction();
}

function getDetectionFrame() {
	const userAction = async () => {
		const response = await fetch('http://0.0.0.0:8081/detected', {
			method: "GET", mode: 'cors',
		});
		const imgBlob = await response.blob();
		let reader = new FileReader();
		reader.readAsDataURL(imgBlob);
		reader.onloadend = function() {
			var base64data = reader.result;
			guiUpdateDetectionStream(base64data);
		}
	}
	userAction();
}

function guiUpdateLiveStream(imgUrl) {
	document.getElementById("live-stream-container").src = imgUrl;
}

function guiUpdateDetectionStream(imgUrl) {
	document.getElementById("detection-image-container").src = imgUrl;
	if (imgUrl != null) {
		guiShowDetectionText();
	}
}

function guiShowDetectionText() {
	document.getElementsByClassName("blink")[0].style.display = 'block';
}

function guiOffDetectionText() {
	document.getElementsByClassName("blink")[0].style.display = 'none';
}

function closeCurtain() {
	console.log('close curtain');
	const userAction = async () => {
		const response = await fetch('http://192.168.1.0/', {
			method: "GET", mode: 'cors',
		});
	}
	userAction();
}

function openCurtain() {
	console.log('open curtain');
	const userAction = async () => {
		const response = await fetch('http://192.168.1.0/counter', {
			method: "GET", mode: 'cors',
		});
	}
	userAction();
}

(function() {
    document.getElementById("close-curtain-button").addEventListener("click", closeCurtain);
	document.getElementById("open-curtain-button").addEventListener("click", openCurtain);
	setInterval(getLiveFrame, 3000);
	setInterval(getDetectionFrame, 3000);
})();

