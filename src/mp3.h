#ifndef MP3_H
#define MP3_H
#include <QString>


class MP3
{
public:
    MP3();
    MP3(QString artist, QString track, QString url, QString session);

    QString Artist;
    QString Track;
    QString Url;
    QString Session;
    QString ToString();
};

#endif // MP3_H
