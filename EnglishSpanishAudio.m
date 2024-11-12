% Set up Arduino connection
arduinoObj = arduino('/dev/tty.usbserial-110', 'Uno');

% Define pins
redButtonPin = 'D2';
blueButtonPin = 'D4';

% Specify audio file paths
englishAudio = 'english.mp3';
spanishAudio = 'spanish.mp3';

% Ensure the buttons are configured as digital input
configurePin(arduinoObj, redButtonPin, 'DigitalInput');
configurePin(arduinoObj, blueButtonPin, 'DigitalInput');

disp('Press the red or blue button to play audio.');

% Loop to continuously check button states
while true
    % Check if blue button is pressed
    if readDigitalPin(arduinoObj, blueButtonPin) == 0
        disp('Blue button pressed - Playing English audio');
        [y, Fs] = audioread(englishAudio);  % Load English audio file
        sound(y, Fs);  % Play English audio
        pause(length(y)/Fs + 1);  % Wait for audio to finish
    end
    
    % Check if red button is pressed
    if readDigitalPin(arduinoObj, redButtonPin) == 0
        disp('Red button pressed - Playing Spanish audio');
        [y, Fs] = audioread(spanishAudio);  % Load Spanish audio file
        sound(y, Fs);  % Play Spanish audio
        pause(length(y)/Fs + 1);  % Wait for audio to finish
    end
    
    % Small delay to prevent excessive CPU usage
    pause(0.1);
end
