% Clear command window and workspace
clc;
clear;

% Load Audio Files
audioEnglish = 'english.mp3';
audioSpanish = 'spanish.mp3';

disp('Press keys 1 for English, 2 for Spanish, or a for Start Game.');

while true
    pause(0.1); % Small pause to avoid CPU overload
    key = input('Press a key: ', 's'); % Prompt user input as a string

    switch key
        case '1'
            % Play English audio
            [y, Fs] = audioread(audioEnglish);
            sound(y, Fs);
            disp('Playing English audio...');

        case '2'
            % Play Spanish audio
            [y, Fs] = audioread(audioSpanish);
            sound(y, Fs);
            disp('Playing Spanish audio...');

        case 'a'
            % Display the "game_start.jpg" image
            gameStartImage = imread('game_start.jpg');
            figure;
            imshow(gameStartImage);
            title('Start Game');
            disp('Displaying Start Game screen...');

        otherwise
            disp('Invalid key pressed. Press 1, 2, or a.');
    end
end
