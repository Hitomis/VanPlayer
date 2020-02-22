//
// Created by 赵帆 on 2020-02-14.
//

#include "IDemux.h"
#include "XLog.h"

void IDemux::run() {
    while (!isExit) {
        if (isPause()) {
            XSleep(2);
            continue;
        }
        XData data = read();
        if (data.size > 0) {
            notify(data);
        } else {
            XSleep(2);
        }
    }
}
