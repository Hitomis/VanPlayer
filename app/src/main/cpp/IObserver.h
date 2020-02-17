//
// Created by 赵帆 on 2020-02-14.
//

#ifndef VANPLAYER_IOBSERVER_H
#define VANPLAYER_IOBSERVER_H


#include "XData.h"
#include "XThread.h"
#include <vector>
#include <mutex>

class IObserver : public XThread {
public:
    virtual void update(XData &data) {}

    void addObserver(IObserver *observer);

    void notify(XData &data);

private:
    std::vector<IObserver *> observers;
    std::mutex mux;
};


#endif //VANPLAYER_IOBSERVER_H
