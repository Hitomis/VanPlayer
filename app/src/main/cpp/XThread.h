//
// Created by 赵帆 on 2020-02-14.
//

#ifndef VANPLAYER_XTHREAD_H
#define VANPLAYER_XTHREAD_H

void XSleep(int timeMillis);

class XThread {
public:
    virtual void start();

    virtual bool isPause();

    virtual void stop();

    virtual void pause(bool flag);

    virtual void run() {}

protected:
    bool isExit = false;
    bool isRunning = false;
    bool isPaused = false;
    bool isPausing = false;

private:
    void threadMain();
};


#endif //VANPLAYER_XTHREAD_H
