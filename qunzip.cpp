#include <linux/limits.h>

#include <QDir>
#include <QFile>
#include <QDebug>
#include "unzip/unzip.h"
#include "qunzip.h"

#define UNZ_ERR ( -120 )

// for original do_extract_currentfile see: miniunz.c
static int do_extract_currentfile( unzFile uf, const QString &targetDir )
{
    char zip_file_path[PATH_MAX];
    char *file_name_withoutpath;
    int err=UNZ_OK;
    char buf[4096]; // file read buffer
    unz_file_info zip_file_info;

    err = unzGetCurrentFileInfo( uf, &zip_file_info, zip_file_path, sizeof( zip_file_path ), NULL, 0, NULL, 0 );
    if (err!=UNZ_OK) {
        qCritical() << "unzGetCurrentFileInfo returned: %d" << err;
        return err;
    }

    QString zipFilePath = QString::fromLocal8Bit( zip_file_path );
    QString targetPath = targetDir + zipFilePath;
    QString fileName = zipFilePath.split( "/" ).last();

    if ( fileName.isEmpty() ) // is a directory
    {
        qDebug() << "creating directory:" << targetPath;
        qDebug("creating directory: %s", zip_file_path );
        QDir().mkpath( targetPath );
        return UNZ_OK;
    }

    err = unzOpenCurrentFile( uf );
    if ( err != UNZ_OK ) {
        qCritical() << "unzOpenCurrentFile returned:" << err;
        return err;
    }

    qDebug() << "Try to open: " << targetPath;
    QFile fo( targetPath );

    // check target path directory existance
    if ( !QDir().mkpath( QFileInfo( targetPath ).path() ) ) {
        qCritical() << "can't create target dir:" << targetDir;
    }

    if ( !fo.open( QIODevice::WriteOnly ) || !fo.isWritable() ) {
        qCritical() << "can't open target file:" << targetPath;
	return UNZ_ERR;
    }

            /* some zipfile don't contain directory alone before file */
            //TODO: handle it ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//            if ((fout==NULL) && filename_withoutpath!=filename)
//            {
//                char c=*(filename_withoutpath-1);
//                *(filename_withoutpath-1)='\0';
//                makedir(write_filename);
//                *(filename_withoutpath-1)=c;
//                fout=fopen(write_filename,"wb");
//            }

    qDebug() << "extracting:" << zipFilePath;

    do {
	err = unzReadCurrentFile( uf, buf, sizeof( buf ) );

        if ( err < 0 ) {
	    qCritical() << "unzReadCurrentFile returned:" << err;
            break;
        }

        if ( err > 0 && fo.write( buf, err ) != err ) {
	    qCritical() << "error in writing extracted file";
            err = UNZ_ERRNO;
            break;
        }

            //TODO: change extracted file's modification date (do we need it?)
//            if (err==0)
//                change_file_date(write_filename,file_info.dosDate,
//                                 file_info.tmu_date);
    } while( err > 0 );

    fo.close();

    err = unzCloseCurrentFile( uf );
    if ( err != UNZ_OK ) {
        qCritical() << "unzCloseCurrentFile returned %d" << err;
        return err;
    }

    return UNZ_OK;
}

void qUnzip(QString archPath, QString targetDir)
{
    unzFile uf = unzOpen( archPath.toLocal8Bit().data() );
    if( uf == NULL ) {
        qCritical() << "can't open archive:" << archPath;
	return;
    }

    if( unzOpenCurrentFile( uf ) != UNZ_OK ) {
        qCritical() << "can't get the first file in archive";
    }

    unz_global_info gi;
    if( unzGetGlobalInfo( uf, &gi ) != UNZ_OK ) {
        qCritical() << "can't get global info";
    }

    int err = unzGoToFirstFile( uf );
    while( err == UNZ_OK ) {
	if ( do_extract_currentfile( uf, targetDir ) != UNZ_OK ) {
	    unzClose( uf );
	    return;
	}
	err = unzGoToNextFile( uf );
    }
}
