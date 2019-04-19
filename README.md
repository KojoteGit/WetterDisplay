# WetterDisplay
Display current data from Netatmo weather station on display using ESP8266

This project is divided into two parts.

## Part 1: PHP script for retrieving data from Netatmo API
The PHP script should be running and accessible in the same network where the ESP is running. It could be located outside of the current network, but take care about security. The script is containing the credentials for accessing the API.
The script is requesting the data from the [Netatmo API](https://dev.netatmo.com) and return it as a concatenated string. Since it's complicated to implement a SSL handling into the ESP8266, the script should be called with HTTP. The request to Netatmo is using SSL.

## Part2: ESP8266 Arduino Code
The code is requesting the current data from the PHP script and displaying the values on the display.
As display a 16x2 display with an I2C converter is used (see circuit). To switch the backlight a button is used.
The backlight will be active for 10 minutes (can be set in the source code to another value).
The pages are cycled after a defined time to display all available values.

To fulfill the requirements of the Netatmo API, an update of the data is requested after 10 minutes.
Please note, your Netatmo weather station is sending the current data every 10 minutes only.

### Used libraries
For debug purposes I used [RemoteDebug](https://github.com/JoaoLopesF/RemoteDebug).

The button control is handled by [JC Button](https://github.com/JChristensen/JC_Button).

For easy handling of the 16x2 LCD the library [LiquidCrystal_I2C](https://github.com/johnrickman/LiquidCrystal_I2C) is required.

## Notes:
The complete code was created on several evenings without claiming big quality. If you want to improve the code, I would be happy if you share it by using Pull Requests.
