% Define the COM port connected to your Arduino
comPort = 'COM3'; % Replace with your actual COM port

% Create an Arduino object
a = arduino(comPort, 'Uno');

% Set up LED pins
BLUE_LED_PIN = 'D6';
RED_LED_PIN = 'D7';
configurePin(a, BLUE_LED_PIN, 'DigitalOutput');
configurePin(a, RED_LED_PIN, 'DigitalOutput');

% Create serial port object
s = serialport(comPort, 9600);

% Allowed access tags
allowedTags = ["0415917AE20F29", "0415915A637D29", "41591CA641728", "415915A", "415917A", "41591CA"];

disp('--------------------------');
disp(' Access Control ');
disp('Place Your Card on the Reader');

cardPresent = false;

while true
    if s.NumBytesAvailable > 0
        % Read the tag ID from the serial port
        tagIDStr = readline(s);
        tagIDStr = strtrim(tagIDStr); % Remove any whitespace

        fprintf('Tag ID: %s\n', tagIDStr);

        % Check if the tag is allowed
        accessGranted = any(strcmp(tagIDStr, allowedTags));

        % Control LEDs based on access
        if accessGranted
            disp(' Access Granted!');
            writeDigitalPin(a, BLUE_LED_PIN, 1);
            writeDigitalPin(a, RED_LED_PIN, 0);
        else
            disp(' Access Denied!');
            writeDigitalPin(a, BLUE_LED_PIN, 0);
            writeDigitalPin(a, RED_LED_PIN, 1);
        end

        % Reset LEDs after a delay
        pause(2); % Wait for 2 seconds
        writeDigitalPin(a, BLUE_LED_PIN, 0);
        writeDigitalPin(a, RED_LED_PIN, 0);

        disp('--------------------------');
        disp('Place Your Card on the Reader');
    end

    pause(0.1); % Small delay to prevent CPU overload
end
