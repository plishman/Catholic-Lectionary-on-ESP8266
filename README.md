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
Several Toolchain projects are provided, written in C# and C++, one of which - TheDotFactory - is not mine, but I have modified it to output "disk fonts", which are bitmap fonts which can be used as an alternative to the built-in rom font used by default. (They may be useful for other character sets needing more detail and larger characters, such as Chinese or Arabic).
<p>
