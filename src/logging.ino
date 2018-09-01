#include "logging.h"
#include "time.h"

// log message text
char logTxt[1024];

String getLogText() {
    return String(logTxt);
}

void logMsg(String msg) {
    String text = getFormattedTime() + " " + msg;
    Serial.println(text);
    
    int len = text.length();
    if (len > 0) {
        int oldLen = strlen(logTxt);

        if (oldLen > 0) {
            if (len + 1 + oldLen > 1023) {
                memmove(logTxt + len + 1, logTxt, 1023 - len);
                logTxt[1023] = '\0';
            } else {
                memmove(logTxt + len + 1, logTxt, oldLen);
                logTxt[len + 1 + oldLen] = '\0';
            }
            logTxt[len] = '\n';
        }

        strncpy(logTxt, text.c_str(), len);
    }
}