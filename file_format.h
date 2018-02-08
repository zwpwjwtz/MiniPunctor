#ifndef FILE_FORMAT_H
#define FILE_FORMAT_H

// Position of record separator
#define PUNCTOR_FILE_REC_SEP_BEFORE 1
#define PUNCTOR_FILE_REC_SEP_AFTER 2
#define PUNCTOR_FILE_REC_SEP_INSERT 3

// Placeholders for record fields
#define PUNCTOR_FILE_REC_VAR_ID "%ID%"
#define PUNCTOR_FILE_REC_VAR_TIME1 "%TIME1%"
#define PUNCTOR_FILE_REC_VAR_TIME2 "%TIME2%"
#define PUNCTOR_FILE_REC_VAR_CONTENT "%CONT%"

// Parameters for pharsing different types of file
#define PUNCTOR_FILE_FORMAT_PLAIN_SEP "\n"
#define PUNCTOR_FILE_FORMAT_PLAIN_SEPPOS PUNCTOR_FILE_REC_SEP_INSERT
#define PUNCTOR_FILE_FORMAT_PLAIN_RECORD "%TIME1%"
#define PUNCTOR_FILE_FORMAT_PLAIN_TIME "hh:mm:ss.zzz"

#define PUNCTOR_FILE_FORMAT_LRC_SEP "\n"
#define PUNCTOR_FILE_FORMAT_LRC_SEPPOS PUNCTOR_FILE_REC_SEP_AFTER
#define PUNCTOR_FILE_FORMAT_LRC_RECORD "[%TIME1%]%CONT%"
#define PUNCTOR_FILE_FORMAT_LRC_TIME "mm:ss.zz"

#define PUNCTOR_FILE_FORMAT_SRT_SEP "\n\n"
#define PUNCTOR_FILE_FORMAT_SRT_SEPPOS PUNCTOR_FILE_REC_SEP_AFTER
#define PUNCTOR_FILE_FORMAT_SRT_RECORD "%ID%\n%TIME1% --> %TIME2%\n%CONT%"
#define PUNCTOR_FILE_FORMAT_SRT_TIME "hh:mm:ss,zzz"

#define PUNCTOR_FILE_FORMAT_SMI_HEADER "<SAMI>\n<BODY>"
#define PUNCTOR_FILE_FORMAT_SMI_FOOTER "</BODY>\n</SAMI>"
#define PUNCTOR_FILE_FORMAT_SMI_BODYBEGIN "<BODY>"
#define PUNCTOR_FILE_FORMAT_SMI_BODYEND "</BODY>"
#define PUNCTOR_FILE_FORMAT_SMI_SEP "<SYNC "
#define PUNCTOR_FILE_FORMAT_SMI_SEPPOS PUNCTOR_FILE_REC_SEP_BEFORE
#define PUNCTOR_FILE_FORMAT_SMI_RECORD "Start=%TIME1%>%CONT%"
#define PUNCTOR_FILE_FORMAT_SMI_TIME "Z"

#define PUNCTOR_FILE_FORMAT_SSA_HEADER \
        "[Script Info]\n"\
        "ScriptType: v4.00\n"\
        "Title: <untitled>\n"\
        "Original Script: <unknown>\n\n"\
        "[V4 Styles]\nFormat: Name, Fontname, Fontsize, "\
        "PrimaryColour, SecondaryColour, TertiaryColour, BackColour, "\
        "Bold, Italic, BorderStyle, Outline, Shadow, Alignment, "\
        "MarginL, MarginR, MarginV, AlphaLevel, Encoding\n\n"\
        "[Events]\n"\
        "Format: Marked, Start, End, Style, Name, "\
        "MarginL, MarginR, MarginV, Effect, Text\n\n"
#define PUNCTOR_FILE_FORMAT_SSA_BODYBEGIN "Dialogue: "
#define PUNCTOR_FILE_FORMAT_SSA_SEP "Dialogue: "
#define PUNCTOR_FILE_FORMAT_SSA_SEPPOS PUNCTOR_FILE_REC_SEP_INSERT
#define PUNCTOR_FILE_FORMAT_SSA_RECORD "Marked=0,%TIME1%,%TIME2%,%CONT%\n"
#define PUNCTOR_FILE_FORMAT_SSA_TIME "H:mm:ss.zz"

#endif // FILE_FORMAT_H
