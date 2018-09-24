# Catholic-Lectionary-on-ESP8266
The Roman Catholic Lectionary, Liturgical Calendar and Feast Days on ESP8266 with Waveshare 2.7" red/black display, generic SD card reader and DS3231 clock IC

<h2>Catholic Lectionary</h2>
<p>
This project implements a Calendar, Liturgical calendar, Lectionary and Feast Day (Sanctorale) calendar on the ESP8266. It also shows an indicator for Holy Days of Obligation.
It is based on the Ruby program calendarium-romanum (https://github.com/igneus/calendarium-romanum) which implements just the Liturgical calendar and Sanctorale days. As a result, it uses the same configuration files and file formats as this program, with some modifications to support month names etc.
</p>

<h3>Localization</h3>
<p>
The project is localized for English, French, Czech and Latin, and other languages, Bible translations, Sanctorale days and Liturgical days can be added over time as all configuration data resides on the SD card.
Arabic ligaturization is supported, and in the SD card archive an Arabic Bible is available (which I was using to test Arabic ligaturization). However, the Sanctorale and Liturgical days are still shown in English with this Bible (but this can be modified if anyone would like to translate the underlying files into Arabic).
</p>

<h3>ToolChain</h3>
<p>
Several Toolchain projects are provided, written in C# and C++, one of which - TheDotFactory (https://github.com/pavius/the-dot-factory/tree/master/TheDotFactory) - is not mine, but I have modified it to output "disk fonts", which are bitmap fonts which can be used as an alternative to the built-in rom font used by default. (They may be useful for other character sets needing more detail and larger characters, such as Chinese or Arabic).
<p>

<h3>Debug Output</h3>
<p>
In order to maximize available pins on the ESP8266, the TX and RX pins are used as an I2C bus to control the DS3231 clock chip. So, to retain the ability to output backchannel debug text therefore an additional program is provided which can be compiled and uploaded to a separate 3.3v Arduino Leonardo, which can then be attached to the ESP8266 by connecting TX to SDA and RX to SCL, and enabling debug output through the web page served from the ESP8266 to configure the Lectionary and Bible translation to be used. (Provide the argument "?debug=1" to the url to which the initial config page is submitted).
</p>

<h3>Hardware</h3>
The current printed circuit board schematic and design can be found at https://easyeda.com/plishman/lectionary_2-3

<p>
There is a lot to this project, and it has taken me over a year to develop the software and a board to run it on, so bear with me - I will update the readme as I go.
</p>
[This readme is still in the process of being updated]

Many thanks to the contributors of Calendarium-Romanum, TheDotFactory and all of the Arduino libraries on which this project depends.
