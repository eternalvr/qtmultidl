#include "mp3.h"

MP3::MP3()
{

}

MP3::MP3(QString artist, QString track, QString url, QString session)
{
     Artist = artist;
     Track = track;
     Url = url;
     Session = session;
}

QString MP3::ToString()
{
    return Artist + " - " + Track;
}
