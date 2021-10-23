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

#ifndef RUNNER_H
#define RUNNER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

class Runner : public QObject
{
  Q_OBJECT

  Q_PROPERTY(bool crashed READ crashed NOTIFY crashedChanged)
  Q_PROPERTY(int  exitCode READ exitCode NOTIFY exitCodeChanged)
  Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
  Runner(QString wayland_socket, QObject *parent=nullptr);
  virtual ~Runner();

  Q_INVOKABLE void start();

  bool crashed() const { return m_crashed; }
  int  exitCode() const { return m_exitCode; }
  QString status() const { return m_status; }

signals:
  void crashedChanged(bool crashed);
  void exitCodeChanged(int exitCode);
  void statusChanged();

  void exit();

protected:
  void onError(QProcess::ProcessError error);
  void onFinished(int /*exitCode*/, QProcess::ExitStatus exitStatus);
  void onCheckSession();
  void onStdOut();
  void onStdErr();

protected:
  QProcess   *m_process_session{nullptr};
  QProcess   *m_process_fullui{nullptr};
  QString     m_wayland_socket;

  QString     m_status;
  bool        m_crashed;
  int         m_exitCode;
};

#endif // RUNNER_H
