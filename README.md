This program is supplyed without warranty under the GNU General Public License.

This program was written originally in 2008.
I, Alex Reinking, the original author, updated this program to Qt 5 in February 2013.

Bugs! (Oh no)
-------------

* The bookmarks feature behaves unexpectedly under Windows, placing a mark 7-10 lines above the line clicked.
	Works fine on CentOS 5, openSuSE 10.3, 11.0, 11.1 and Ubuntu 7.10
* Highligher, Priority: Minimal, Description: /* *//This text here is a comment
	LOW priority because it affects only one line, and that this case is so rare that it's easier to just put a space than to fix.

LPSD Documentation
------------------
LPSD files are strictly typed XML files. The formats are located in ./formats and end with a .lpsd extension
The configuration file is in ./config/config.lpsc this file names all of the formats useable in an LPSD document.

LPSC Format
----
Must start with <?xml version="1.0"?> to identify itself as an XML file
The <lpsc> tag takes no parameters and acts as the opening/closing of the file.
The only tag that can be a direct child of <lpsc> is the <format> tag
The <format> tag takes exactly one parameter, id, this is the name for the color. If two names conflict,
  the one defined later will be "effective"
The format tag requires four children. <bold>, <italic>, <underline> and <color>. The first three must be "true" or "false"
  the color tag accepts the predefined colors here: http://doc.trolltech.com/4.4/qcolor.html#predefined-colors and HTML Hex codes (#3399FF)

Example file:
<?xml version="1.0"?>
<lpsc>
	<format id="foo">
		<bold>false</bold>
		<italic>true</italic>
		<underline>true</underline>
		<color>#00ff00</color>
	</format>
</lpsc>


LPSD Format
----
