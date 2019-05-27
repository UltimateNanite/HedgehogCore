void tinyfd_beep();

int tinyfd_notifyPopup(
	char const * const aTitle, // NULL or ""
	char const * const aMessage, // NULL or "" may contain \n \t
	char const * const aIconType); // "info" "warning" "error"

int tinyfd_messageBox(
	char const * const aTitle, // NULL or ""
	char const * const aMessage, // NULL or "" may contain \n \t
	char const * const aDialogType, // "ok" "okcancel" "yesno" "yesnocancel"
	char const * const aIconType, // "info" "warning" "error" "question"
	int const aDefaultButton);
// 0 for cancel/no , 1 for ok/yes , 2 for no in yesnocancel

char const * tinyfd_inputBox(
	char const * const aTitle, // NULL or ""
	char const * const aMessage, // NULL or "" may NOT contain \n \t on windows
	char const * const aDefaultInput); // "" , if NULL it's a passwordBox
									   // returns NULL on cancel

char const * tinyfd_saveFileDialog(
	char const * const aTitle, // NULL or ""
	char const * const aDefaultPathAndFile, // NULL or ""
	int const aNumOfFilterPatterns, // 0
	char const * const * const aFilterPatterns, // NULL | {"*.txt"}
	char const * const aSingleFilterDescription); // NULL | "text files"
												  // returns NULL on cancel

char const * tinyfd_openFileDialog(
	char const * const aTitle, // NULL or ""
	char const * const aDefaultPathAndFile, // NULL or ""
	int const aNumOfFilterPatterns, // 0
	char const * const * const aFilterPatterns, // NULL {"*.jpg","*.png"}
	char const * const aSingleFilterDescription, // NULL | "image files"
	int const aAllowMultipleSelects); // 0
									  // in case of multiple files, the separator is |
									  // returns NULL on cancel

char const * tinyfd_selectFolderDialog(
	char const * const aTitle, // NULL or ""
	char const * const aDefaultPath); // NULL or ""
									  // returns NULL on cancel

char const * tinyfd_colorChooser(
	char const * const aTitle, // NULL or ""
	char const * const aDefaultHexRGB, // NULL or "#FF0000”
	unsigned char const aDefaultRGB[3], // { 0 , 255 , 255 }
	unsigned char aoResultRGB[3]); // { 0 , 0 , 0 }
								   // returns the hexcolor as a string "#FF0000"
								   // aoResultRGB also contains the result
								   // aDefaultRGB is used only if aDefaultHexRGB is NULL
								   // aDefaultRGB and aoResultRGB can be the same array
								   // returns NULL on cancel

