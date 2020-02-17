//
// Created by 赵帆 on 2020-02-14.
//

#ifndef VANPLAYER_XTHREAD_H
#define VANPLAYER_XTHREAD_H

void XSleep(int timeMillis);

class XThread {
public:
    virtual void start();

    virtual void stop();

    virtual void run() {}

protected:
    bool isExit = false;
    bool isRunning = false;

private:
    void threadMain();
};


#endif //VANPLAYER_XTHREAD_H
