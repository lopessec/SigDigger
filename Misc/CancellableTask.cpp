//
//    CancellableTask.cpp: Cancellable asynchronous task
//    Copyright (C) 2020 Gonzalo José Carracedo Carballal
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as
//    published by the Free Software Foundation, either version 3 of the
//    License, or (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful, but
//    WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this program.  If not, see
//    <http://www.gnu.org/licenses/>
//
#include <CancellableTask.h>

using namespace SigDigger;

////////////////////////////// CancellableTask /////////////////////////////////
CancellableTask::CancellableTask(QObject *parent) : QObject(parent)
{

}

CancellableTask::~CancellableTask(void)
{

}

void
CancellableTask::setProgress(qreal progress)
{
  this->prog = progress;
}

void
CancellableTask::setStatus(QString status)
{
  this->status = status;
}

void
CancellableTask::onWorkRequested(void)
{
  if (this->work())
    emit progress(prog, status);
}

void
CancellableTask::onCancelRequested(void)
{
  this->cancel();
}

/////////////////////////// CancellableController //////////////////////////////
CancellableController::CancellableController(QObject *parent) : QObject(parent)
{
  this->worker.start();
}

CancellableController::~CancellableController(void)
{
  emit queuedCancel();
  this->worker.quit();
  this->worker.wait();

  if (task != nullptr)
    delete task;
}

void
CancellableController::connectTask(void)
{
  // Signals from controller to task
  connect(
        this,
        SIGNAL(queuedWork()),
        this->task,
        SLOT(onWorkRequested()));

  connect(
        this,
        SIGNAL(queuedCancel()),
        this->task,
        SLOT(onCancelRequested()));

  // Signals from task to controller
  connect(
        this->task,
        SIGNAL(progress(qreal, QString)),
        this,
        SLOT(onProgress(qreal, QString)));

  connect(
        this->task,
        SIGNAL(done(void)),
        this,
        SLOT(onDone(void)));

  connect(
        this->task,
        SIGNAL(cancelled(void)),
        this,
        SLOT(onCancelled(void)));

  connect(
        this->task,
        SIGNAL(error(QString)),
        this,
        SLOT(onError(QString)));
}

bool
CancellableController::process(QString const &name, CancellableTask *task)
{
  if (this->task != nullptr)
    return false;

  this->name = name;
  this->task = task;
  this->cancelledState = false;

  emit progress(task->getProgress(), task->getStatus());

  task->moveToThread(&this->worker);

  this->connectTask();

  emit queuedWork();

  return true;
}

bool
CancellableController::cancel(void)
{
  if (this->task == nullptr || this->cancelledState)
    return false;

  emit cancelling();
  emit queuedCancel();

  return true;
}

void
CancellableController::onDone(void)
{
  emit done();

  delete this->task;
  this->task = nullptr;
}

void
CancellableController::onCancelled(void)
{
  this->cancelledState = true;
  delete this->task;
  this->task = nullptr;

  emit cancelled();
}

void
CancellableController::onError(QString errmsg)
{
  delete this->task;
  this->task = nullptr;

  emit error(errmsg);
}

void
CancellableController::onProgress(qreal prog, QString status)
{
  emit progress(prog, status);

  if (!this->cancelledState)
    emit queuedWork();
}
