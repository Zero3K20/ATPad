#ifndef	__STRINGCONSTANTS_H_
#define	__STRINGCONSTANTS_H_

#define	MDI_CLIENT_CLASS	L"MDICLIENT"
#define	MDI_CHILD_CLASS		L"Win32MDIChild"
#define	PROGRAM_NAME		L"ATPad"
#define	PROG_ANSI_NAME		"ATPad"
#define	PROG_EXE_NAME		"ATPad.exe"
#define	UPDATE_CHECK_URL	"atpad.sourceforge.net"
#define	HOME_PAGE			L"http://atpad.sf.net"
#define	DOWNLOAD_PAGE		L"http://atpad.sf.net?page=3"
#define	HELP_SUBPATH		L"\\ATPad.chm"
#define	DELIMETER			L"^"
#define	CHAR_DELIMETER		'^'
#define	PROG_CLASS			L"__ATPad__Application__Main__Window__Class__"
#define	PROG_ANSI_CLASS		"__ATPad__Application__Main__Window__Class__"
#define	PROG_MUTEX			L"__ATPad_prog_mutex__"
#define	INI_FILE			L"atpad.ini"
#define	INI_FILE_SUBPATH	L"\\atpad.ini"
#define	SNP_FILE_SUBPATH	L"\\atpad.snps"
#define	S_LANG				L"language"
#define	S_ACCELERATORS		L"accelerators"
#define	S_FILE				L"file"
#define	S_MENU				L"menu"
#define	S_STRINGS			L"strings"
#define	S_PREFERENCES		L"preferences"
#define	S_CONTROLS			L"controls"
#define	S_TABS				L"tabs"
#define	S_LINES_AL			L"line_alignment"
#define	S_RECENT_FILES		L"recent_files"
#define	S_RECENT_BOOKMARKS	L"recent_bookmarks"
#define	S_FIND				L"find"
#define	S_REPLACE			L"replace"
#define	S_PRINT				L"print"
#define	S_LASTSESSION		L"last_session_files"
#define	S_LASTS_BOOKMARKS	L"last_session_bookmarks"
#define	S_REFRESHFREQ		L"refresh_freq"			
#define	S_SNIPPETS_COLS		L"snippets_cols"
#define	S_DT_FORMATS		L"date_time_formats"
#define	S_DF_CHARS			L"date_format_chars"
#define	S_TF_CHARS_H12		L"time_format_chars_h12"
#define	S_TF_CHARS_H24		L"time_format_chars_h24"
#define	S_TF_CHARS			L"time_format_chars"
#define	K_SEARCH_PARAMS		L"search_params"
#define	K_SETTINGS			L"settings"
#define	K_POSITION			L"position"
#define	K_LANGUAGE_FILE		L"language"
#define	K_TEXT_FORMAT		L"text_format"
#define	K_LINES_FORMAT		L"lines_format"
#define	K_PRINT_SETUP		L"print_setup"
#define	K_REFRESH_CAP		L"refresh_caption"
#define	K_REFRESH_SET		L"refresh_set"
#define	K_BREAKS_COLOR		L"breaks_color"
#define	K_TABS_COLOR		L"tabs_color"
#define	K_SPACE_COLOR		L"space_color"
#define	K_BM_COLOR			L"bookmark_color"
#define	K_D_FORMAT			L"date"
#define	K_T_FORMAT			L"time"
#define	K_T_FIRST			L"date_first"
#define	K_T_CURR_SETT		L"use_curr_settings"
#define	ENC1				L"ANSI"
#define	ENC2				L"Unicode"
#define	ENC3				L"Unicode big endian"
#define	ENC4				L"UTF-8"
#define	SAVE_FILTER			L"Text Documents (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0"
#define	EXE_FILTER			L"Executables (*.exe)\0*.exe\0\0"
#define	OPEN_FILTER			L"All files (*.*)\0*.*\0\0"
#define	FONT_NP				L"Courier New"
#define	FONT_LN				L"Tahoma"
#define	P_SAVE_ON_EXIT		L"SAVE_ON_EXIT"
#define	EMAIL_SUBJECT		L"Emailing by ATPad"
#define	REMOVABLE_MAC		L"xx-xx-xx-xx-xx-xx"
//datetime format strings
#define	DTS_DATE_FORMAT		L"dd MMM yyyy, "
#define	DTS_TIME_FORMAT		L"HH:mm"
//preview string
#define	PREVIEW_STRING		L"\t`Well!` thought Alice to herself `After such a fall as this, I shall think nothing of tumbling down-stairs! \rHow brave they'll all think me at home! \rWhy, I wouldn't say anything about it, even if I fell off the top of the house!` (which was very likely true.) "\
								L"\r\tDown, down, down. Would the fall never come to an end? \r`I wonder how many miles I've fallen by this time?` she said aloud. \r`I must be getting somewhere near the centre of the earth. Let me see: that would be four thousand miles down, I think-` \r(for, you see, Alice had learnt several things of this sort in her lessons in the school-room, and though this was not a very good opportunity for showing off her knowledge, as there was no one to listen to her, still it was good practice to say it over) `-- yes that's about the right distance -- but then I wonder what Latitude or Longitude I've got to?` (Alice had not the slightest idea what Latitude was, or Longitude either, but she thought they were nice grand words to say.) "
//datetime help strings
#define	DATE_FORMAT_MASKS	L"d\tDay of month as digits with no leading zero for single-digit days.\n"\
							"dd\tDay of month as digits with leading zero for single-digit days.\n"\
							"ddd\tDay of week as a three-letter abbreviation.\n"\
							"dddd\tDay of week as its full name.\n"\
							"M\tMonth as digits with no leading zero for single-digit months.\n"\
							"MM\tMonth as digits with leading zero for single-digit months.\n"\
							"MMM\tMonth as a three-letter abbreviation.\n"\
							"MMMM\tMonth as its full name.\n"\
							"y\tYear as last two digits, but with no leading zero for years less than 10.\n"\
							"yy\tYear as last two digits, but with leading zero for years less than 10.\n"\
							"yyyy\tYear represented by full four digits."
#define	TIME_FMT_MASKS_H12	L"h\tHours with no leading zero for single-digit hours; 12-hour clock.\n"\
							"hh\tHours with leading zero for single-digit hours; 12-hour clock.\n"
#define	TIME_FMT_MASKS_H24	L"H\tHours with no leading zero for single-digit hours; 24-hour clock.\n"\
							"HH\tHours with leading zero for single-digit hours; 24-hour clock.\n"
#define	TIME_FMT_MASKS		L"m\tMinutes with no leading zero for single-digit minutes.\n"\
							"mm\tMinutes with leading zero for single-digit minutes.\n"\
							"s\tSeconds with no leading zero for single-digit seconds.\n"\
							"ss\tSeconds with leading zero for single-digit seconds.\n"\
							"t\tOne character time-marker string, such as A or P.\n"\
							"tt\tMulticharacter time-marker string, such as AM or PM.\n"
#endif
