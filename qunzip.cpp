#include <QDir>
#include <QFile>
#include <QDebug>
#include "unzip/unzip.h"
#include "qunzip.h"

// for original do_extract_currentfile see: miniunz.c
static int do_extract_currentfile(unzFile uf, const QString &targetDir)
{
    char filename_inzip[256];
    char* filename_withoutpath;
    char* p;
    int err=UNZ_OK;
    char buf[4096]; //file read buffer
    uInt size_buf=4096;
    unz_file_info file_info;

    err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
    if (err!=UNZ_OK) {
        qDebug("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        return err;
    }

    p = filename_withoutpath = filename_inzip;
    while ((*p) != '\0')
    {
        if (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    if ((*filename_withoutpath)=='\0') //is a directory
    {
        qDebug("creating directory: %s\n",filename_inzip);
        QDir d;
        d.mkdir(targetDir + filename_inzip);
    }
    else {
        const char* write_filename;

        write_filename = filename_inzip;

        err = unzOpenCurrentFile(uf);
        if (err!=UNZ_OK) {
            qDebug("error %d with zipfile in unzOpenCurrentFile\n",err);
        }


        QFile fo(targetDir + write_filename);
        qDebug() << "Try to open this: " << targetDir + write_filename;
        if (err==UNZ_OK) {
            fo.open(QIODevice::WriteOnly);

            /* some zipfile don't contain directory alone before file */
            //TODO: handle it ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//            if ((fout==NULL) && filename_withoutpath!=filename_inzip)
//            {
//                char c=*(filename_withoutpath-1);
//                *(filename_withoutpath-1)='\0';
//                makedir(write_filename);
//                *(filename_withoutpath-1)=c;
//                fout=fopen(write_filename,"wb");
//            }

            if (!fo.isOpen()||!fo.isWritable()) {
                qWarning("error opening %s\n",write_filename);
            }
        }

        if (fo.isOpen() && fo.isWritable())
        {
            qDebug(" extracting: %s",write_filename);

            do
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                if (err<0) {
                    qWarning("error %d with zipfile in unzReadCurrentFile\n",err);
                    break;
                }
                if (err>0) //then err is number of bytes read by unzReadCurrentFile
                    if (fo.write(buf, err)!=err) {
                        qWarning("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        break;
                    }
            } while (err>0);
            if(fo.isOpen()) {
                fo.close();
            }
            //TODO: change extracted file's modification date (do we need it?)
//            if (err==0)
//                change_file_date(write_filename,file_info.dosDate,
//                                 file_info.tmu_date);
        }

        if (err==UNZ_OK) {
            err = unzCloseCurrentFile (uf);
            if (err!=UNZ_OK) {
                qWarning("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        else
            unzCloseCurrentFile(uf);
    }
    return err;
}

void qUnzip(QString archPath, QString targetDir)
{
    unzFile uf = unzOpen(archPath.toLocal8Bit().data());
    if(uf!=NULL) {
        if(unzOpenCurrentFile(uf)==UNZ_OK) {            
            unz_global_info gi;
            if(unzGetGlobalInfo (uf,&gi)==UNZ_OK) {
                for (int i=0;i<gi.number_entry;i++)
                {
                    if (do_extract_currentfile(uf,targetDir) != UNZ_OK)
                        break;

                    if ((i+1)<gi.number_entry)
                    {
                        int err = unzGoToNextFile(uf);
                        if (err!=UNZ_OK)
                        {
                            qWarning("error %d with zipfile in unzGoToNextFile\n",err);
                            break;
                        }
                    }
                }
            }
            else {
                qWarning("WARN: unzGetGlobalInfo");
            }
        }
        else {
            qWarning("WARN: can't open zip for read");
        }
    }
    else {
        qWarning("WARN: unzip won't work :(");
    }
    //TODO: return errorcode
}
