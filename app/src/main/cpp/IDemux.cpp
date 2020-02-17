//
// Created by 赵帆 on 2020-02-14.
//

#include "IDemux.h"
#include "XLog.h"

void IDemux::run() {
    while (!isExit) {
        XData data = read();
//        XLOGE("IDemux read %d", data.size);
        if (data.size <= 0) break;
        notify(data);
    }
}
