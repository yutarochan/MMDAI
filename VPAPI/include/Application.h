/**

 Copyright (c) 2010-2014  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef APPLICATIONBOOTSTRAP_H_
#define APPLICATIONBOOTSTRAP_H_

#include <QApplication>
#include <QCommandLineParser>
#include <QUrl>

class Application : public QApplication
{
    Q_OBJECT

public:
    Q_PROPERTY(QString commitRevision READ commitRevision CONSTANT FINAL)
    Q_PROPERTY(QUrl requestedFileUrl READ requestedFileUrl NOTIFY requestedFileUrlChanged FINAL)
    Q_PROPERTY(QUrl json READ json CONSTANT FINAL)
    Q_PROPERTY(bool hasJson READ hasJson CONSTANT FINAL)

    Application(QCommandLineParser *parser, int argc, char **argv);
    ~Application();

    QString commitRevision() const;
    QUrl requestedFileUrl() const;
    QUrl json() const;
    bool hasJson() const;

signals:
    void requestedFileUrlChanged();

protected:
    bool event(QEvent *event);

private:
    const QCommandLineParser *m_parser;
    QCommandLineOption m_json;
    QUrl m_requestedFileUrl;
};

#endif
