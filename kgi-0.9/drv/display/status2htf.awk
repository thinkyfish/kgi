BEGIN {

	first_board   = 1
	first_chipset = 1
	first_clock   = 1
	first_ramdac  = 1
	first_monitor = 1

	end_action = ""
	end_category = ""

	status_color["10"] = "#FF0000"
	status_color["20"] = "#FF3300"
	status_color["30"] = "#FF6600"
	status_color["40"] = "#FF9800"
	status_color["50"] = "#FFCC00"
	status_color["60"] = "#FFFF00"
	status_color["70"] = "#CCFF00"
	status_color["80"] = "#66FF00"
	status_color["90"] = "#33FF00"
	status_color["ok"] = "#00FF00"

	printf	"HTF_LANGUAGE=en\n"
	printf	"HTF_TITLE=\"Display Driver Status\"\n"
	printf	"HTF_DESCRIPTION=\"This page gives a status evaluation of the drivers included with this distribution.\"\n"
	printf	"HTF_KEYWORDS=\"KGI Display Driver; status; supported hardware\"\n"
	printf	"HTF_AUTHOR=\"status2htf.awk\"\n"
	printf	"HTF_REVISION=\"%s\"\n", strftime("%Y-%m-%d/%H:%m")
	printf	"cat <<-end-of-htf\n"
}


/\/vendors/	{ action = "switch-to-vendors"	}
/\/board\//	{ action = "switch-to-board"	}
/\/chipset\//	{ action = "switch-to-chipset"	}
/\/clock\//	{ action = "switch-to-clock"	}
/\/ramdac\//	{ action = "switch-to-ramdac"	}
/\/monitor\//	{ action = "switch-to-monitor"	}

{
	if (action == "parse-vendors") {

		vendor_name[$1] = $2
		vendor_url[$1] = $3
	}
	if ((action == "parse-board") || 
		(action == "parse-chipset") ||
		(action == "parse-clock") ||
		(action == "parse-ramdac") ||
		(action == "parse-monitor")) {

		if ($3 == "ok") {

			status_text = $3

		} else {

			status_text = $3 "%"
		}

		printf	"\t<tr>"
		printf	"<td>%s</td>\t", $2
		printf	"<td bgcolor=\"%s\" align=center>%s</td>\t", 
			status_color[$3], status_text
		printf	"<td>%s</td>\t", $4
		printf	"</tr>\n"
	}

	#
	#	state transition
	#
	if (action == "switch-to-vendors") {

		action = "parse-vendors"
		end_action = ""
	}

	if (action == "switch-to-board") {

		printf	"%s\n", end_action

		if (first_board) {

			printf	"%s\n", end_category
			printf	"<a name=\"board\"></a>\n"
			printf	"<h1>Supported Boards by Vendor</h1>\n"
			printf	"<blockquote><dl>\n"
			first_board = 0
			end_category = "</dl></blockquote>"
		}

		sub(/[^b]*board\//, "")
		gsub(/\//, "\t")

		printf	"\t<dt>"
		if (vendor_url[$1]) {

			printf	"<a href=\"%s\" target=\"_top\">%s</a>\n",
				vendor_url[$1], vendor_name[$1]
		} else {
			printf	"%s\n", vendor_name[$1]
		}
		printf	"\t<dd><table bgcolor=\"#FFFFCC\" nosave width=\"80%%\">\n"
		printf	"\t<tr bgcolor=\"#CCFFFF\">"
		printf	"<td width=\"60%%\">Board Model</td>"
		printf	"<td width=\"10%%\">Status</td>"
		printf	"<td width=\"30%%\">Maintainer</td>"
		printf	"</tr>\n"

		action = "parse-board"
		end_action = "\t</table><br></dt>"
	}

	if (action == "switch-to-chipset") {

		printf "%s\n", end_action

		if (first_chipset) {

			printf	"%s\n", end_category
			printf	"<a name=\"chipset\"></a>\n"
			printf	"<h1>Supported Chipsets by Vendor</h1>\n"
			printf	"<blockquote><dl>\n"
			first_chipset = 0
			end_category = "</dl></blockquote>"
		}

		sub(/[^c]*chipset\//, "")
		gsub(/\//, "\t")

		printf	"\t<dt>"
		if (vendor_url[$1]) {

			printf	"<a href=\"%s\" target=\"_top\">%s</a>\n",
				vendor_url[$1], vendor_name[$1]
		} else {
			printf	"%s\n", vendor_name[$1]
		}
		printf	"\t<dd><table bgcolor=\"#FFFFCC\" nosave width=\"80%%\">\n"
		printf	"\t<tr bgcolor=\"#CCFFFF\">"
		printf	"<td width=\"60%%\">Chipset Model</td>"
		printf	"<td width=\"10%%\">Status</td>"
		printf	"<td width=\"30%%\">Maintainer</td>"
		printf	"</tr>\n"

		action = "parse-chipset"
		end_action = "\t</table><br></dt>"
	}

	if (action == "switch-to-clock") {

		printf "%s\n", end_action

		if (first_clock) {

			printf	"%s\n", end_category
			printf	"<a name=\"clock\"></a>\n"
			printf	"<h1>Supported Clock Chips by Vendor</h1>\n"
			printf	"<blockquote><dl>\n"
			first_clock = 0
			end_category = "</dl></blockquote>"
		}

		sub(/[^c]*clock\//, "")
		gsub(/\//, "\t")

		printf	"\t<dt>"
		if (vendor_url[$1]) {

			printf	"<a href=\"%s\" target=\"_top\">%s</a>\n",
				vendor_url[$1], vendor_name[$1]
		} else {
			printf	"%s\n", vendor_name[$1]
		}
		printf	"\t<dd><table bgcolor=\"#FFFFCC\" nosave width=\"80%%\">\n"
		printf	"\t<tr bgcolor=\"#CCFFFF\">"
		printf	"<td width=\"60%%\">Clock Chip Model</td>"
		printf	"<td width=\"10%%\">Status</td>"
		printf	"<td width=\"30%%\">Maintainer</td>"
		printf	"</tr>\n"

		action = "parse-clock"
		end_action = "\t</table><br></dt>"
	}

	if (action == "switch-to-ramdac") {

		printf "%s\n", end_action

		if (first_ramdac) {

			printf	"%s\n", end_category
			printf	"<a name=\"ramdac\"></a>\n"
			printf	"<h1>Supported DAC Chips by Vendor</h1>\n"
			printf	"<blockquote><dl>\n"
			first_ramdac = 0
			end_category = "</dl></blockquote>"
		}

		sub(/[^c]*ramdac\//, "")
		gsub(/\//, "\t")

		printf	"\t<dt>"
		if (vendor_url[$1]) {

			printf	"<a href=\"%s\" target=\"_top\">%s</a>\n",
				vendor_url[$1], vendor_name[$1]
		} else {
			printf	"%s\n", vendor_name[$1]
		}
		printf	"\t<dd><table bgcolor=\"#FFFFCC\" nosave width=\"80%%\">\n"
		printf	"\t<tr bgcolor=\"#CCFFFF\">"
		printf	"<td width=\"60%%\">DAC Chip Model</td>"
		printf	"<td width=\"10%%\">Status</td>"
		printf	"<td width=\"30%%\">Maintainer</td>"
		printf	"</tr>\n"

		action = "parse-ramdac"
		end_action = "\t</table><br></dt>"
	}

	if (action == "switch-to-monitor") {

		printf "%s\n", end_action

		if (first_monitor) {

			printf	"%s\n", end_category
			printf	"<a name=\"monitor\"></a>\n"
			printf	"<h1>Supported Monitors by Vendor</h1>\n"
			printf	"<blockquote><dl>\n"
			first_monitor = 0
			end_category = "</dl></blockquote>"
		}

		sub(/[^c]*monitor\//, "")
		gsub(/\//, "\t")

		printf	"\t<dt>"
		if (vendor_url[$1]) {

			printf	"<a href=\"%s\" target=\"_top\">%s</a>\n",
				vendor_url[$1], vendor_name[$1]
		} else {
			printf	"%s\n", vendor_name[$1]
		}
		printf	"\t<dd><table bgcolor=\"#FFFFCC\" nosave width=\"80%%\">\n"
		printf	"\t<tr bgcolor=\"#CCFFFF\">"
		printf	"<td width=\"60%%\">Monitor Model</td>"
		printf	"<td width=\"10%%\">Status</td>"
		printf	"<td width=\"30%%\">Maintainer</td>"
		printf	"</tr>\n"

		action = "parse-monitor"
		end_action = "\t</table><br></dt>"
	}
}

END {
	printf	"%s\n", end_action
	printf	"%s\n", end_category

	printf	"end-of-htf\n"
}
