/#define XF86_VERSION_MAJOR/	{ X11_MAJOR=$3 }
/#define XF86_VERSION_MINOR/	{ X11_MINOR=$3 }
/#define XF86_VERSION_SUBMINOR/	{ X11_PATCH=$3 }
/#define XF86_VERSION_BETA/	{ X11_BETA=$3 }
/#define XF86_VERSION_ALPHA/	{ X11_ALPHA=$3 }

END {
	printf	"X11_VERSION=%s.%s.%s-%s.%s\n", 
			X11_MAJOR, X11_MINOR, X11_PATCH,
			X11_BETA, X11_ALPHA
	printf	"X11_MAJOR=%s;\n", X11_MAJOR
	printf	"X11_MINOR=%s;\n", X11_MINOR
	printf	"X11_PATCH=%s;\n", X11_PATCH
	printf	"X11_EXTRA=-%s.%s;\n", X11_BETA, X11_ALPHA

	if ((X11_MAJOR < 3) ||
		((X11_MAJOR == 3) && (X11_MINOR < 9))) {

		printf	"X11_XC_PATCH=notanylonger;\n"
		exit 0;
	}

	if ((X11_MAJOR > 3) ||
		((X11_MAJOR == 3) && (X11_MINOR > 9))) {

		printf	"X11_XC_PATCH=notyet;\n"
		exit 0
	}

	if (X11_PATCH < 18) {

		printf	"X11_XC_PATCH=notanylonger;\n"
		exit 0
	}

	X11_XC_PATCH = "3.9.18"

	printf	"X11_XC_PATCH=%s;\n", X11_XC_PATCH
}
