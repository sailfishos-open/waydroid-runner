/****************************************************************************
**
** Copyright (C) 2020 Rinigus https://github.com/rinigus
**
** This file is part of Flatpak Runner.
**
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of the copyright holder nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/

#include "runner.h"

#include <QProcessEnvironment>
#include <QSettings>
#include <QTimer>

#include <iostream>

#include <QDebug>

#ifndef WAYDROID_PATH
#define WAYDROID_PATH "/usr/bin/waydroid"
#endif

Runner::Runner(QString wayland_socket, QObject *parent):
  QObject(parent),
  m_wayland_socket(wayland_socket)
{
  m_status = tr("Initializing");
  m_status_code = Status::Idle;
  // Wayland
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("WAYLAND_DISPLAY", wayland_socket);

  // prepare processes
  m_process_session = new QProcess(this);
  m_process_session->setProcessEnvironment(env);

  connect(m_process_session, &QProcess::errorOccurred, this, &Runner::onError);
  connect(m_process_session, &QProcess::readyReadStandardError, this, &Runner::onStdErr);
  connect(m_process_session, &QProcess::readyReadStandardOutput, this, &Runner::onStdOut);
  connect(m_process_session, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
          this, &Runner::onFinished);
  connect(m_process_session,&QProcess::started, this, &Runner::onCheckSession);

  m_process_fullui = new QProcess(this);
  m_process_fullui->setProcessEnvironment(env);

  connect(m_process_fullui, &QProcess::readyReadStandardError, this, &Runner::onStdErr);
  connect(m_process_fullui, &QProcess::readyReadStandardOutput, this, &Runner::onStdOut);
}

Runner::~Runner()
{
  std::cout << "Closing Android session" << std::endl;
  if (m_process_session && m_process_session->state() == QProcess::Running)
    {
      QProcess::execute(WAYDROID_PATH, QStringList() << "session" << "stop");
      m_process_session->waitForFinished(3000);
    }

  if (m_process_fullui)
    {
      if (m_process_fullui->state() == QProcess::Running)
        m_process_fullui->waitForFinished(3000);
    }
}

void Runner::start()
{
  // shouldn't happen
  if (!m_process_session)
    {
      m_status = tr("Session process not available, cannot start");
      m_status_code = Status::ErrorSessionNotAvailable;
      emit statusChanged();
      return;
    }

  checkStatus();
  if (m_status_session_running)
    {
      m_status = tr("Android session started already. Stop that session and restart this application.");
      m_status_code = Status::ErrorSessionRunning;
      emit statusChanged();
      return;
    }

  m_process_session->start(WAYDROID_PATH, QStringList() << "session" << "start");
  m_status = tr("Starting Android session");
  m_status_code = Status::StartingSession;
  emit statusChanged();
}

void Runner::stopSession()
{
  m_process_session->start(WAYDROID_PATH, QStringList() << "session" << "stop");
  m_status = tr("Stopping Android session");
  m_status_code = Status::StoppingSession;
  emit statusChanged();
}

void Runner::checkStatus()
{
  QProcess check;
  m_status_container_running = false;
  m_status_session_running = false;
  m_status_wayland_socket = QString();

  check.start(WAYDROID_PATH, QStringList() << "status");
  check.waitForFinished();
  auto stdout = check.readAllStandardOutput().split('\n');
  QString wd = QStringLiteral("Wayland display:");
  for (QString line: stdout)
    {
      if (line.indexOf("Container:") >= 0 && line.indexOf("RUNNING") > 0)
        m_status_container_running = true;
      if (line.indexOf("Session:") >= 0 && line.indexOf("RUNNING") > 0)
        m_status_session_running = true;
      int wdi = line.indexOf(wd);
      if (wdi >= 0)
        m_status_wayland_socket = line.mid(wdi + wd.length()).trimmed();
    }
}

void Runner::onCheckSession()
{
  // check if session is running
  checkStatus();
  if (m_status_session_running && m_status_container_running)
    {
      qDebug() << "Current Wayland socket: " << m_status_wayland_socket;
      if (m_wayland_socket != m_status_wayland_socket)
        {
          std::cerr << "Android session running using Wayland display: " << m_status_wayland_socket.toStdString() << "\n"
                    << "Expected value: " << m_wayland_socket.toStdString() << std::endl;
          m_status = tr("Unexpected Wayland display setting for running Android session. Stopping the execution.");
          m_status_code = Status::ErrorUnexpected;
          emit statusChanged();
          return;
        }

      // this called only once as timer single shots will not be requested
      m_process_fullui->start(WAYDROID_PATH, QStringList() << "show-full-ui");
      m_status = tr("Waiting for Android UI");
      m_status_code = Status::WaitingForUI;
      emit statusChanged();
    }
  else
    QTimer::singleShot(2000, this, &Runner::onCheckSession);
}

void Runner::onError(QProcess::ProcessError /*error*/)
{
  std::cerr << m_process_session->errorString().toStdString() << "\n";
}

void Runner::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  onStdErr();
  onStdOut();

  bool c = (exitStatus == QProcess::CrashExit);
  if (m_crashed != c)
    {
      m_crashed = c;
      emit crashedChanged(m_crashed);
    }

  if (m_exitCode != exitCode)
    {
      m_exitCode = exitCode;
      emit exitCodeChanged(m_exitCode);
    }

  if (m_crashed) {
    m_status = tr("Android session crashed");
    m_status_code = Status::ErrorCrashed;
  } else if (m_exitCode) {
    m_status = tr("Android session finished with the exit code %1").arg(m_exitCode);
    m_status_code = Status::ErrorExited;
  } else {
    m_status = tr("Android session finished");
    m_status_code = Status::Idle;
  }
  emit statusChanged();
  emit exit();
}

static void output(bool err, const char *prefix, QProcess *process)
{
  if (!process || !process->isReadable())
    return;

  QByteArray txt = err ? process->readAllStandardError() : process->readAllStandardOutput();
  if (!txt.isEmpty())
    {
      if (err)
        std::cerr << prefix << " " << txt.constData() << std::flush;
      else
        std::cout << prefix << " " << txt.constData() << std::flush;
    }
}

void Runner::onStdErr()
{
  if (m_process_session)
    output(true, "[Session]", m_process_session);
  if (m_process_fullui)
    output(true, "[FullUI]", m_process_fullui);
}

void Runner::onStdOut()
{
  if (m_process_session)
    output(false, "[Session]", m_process_session);
  if (m_process_fullui)
    output(false, "[FullUI]", m_process_fullui);
}
