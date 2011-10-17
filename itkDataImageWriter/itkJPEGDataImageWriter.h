#ifndef ITKJPEGDATAIMAGEWRITER_H
#define ITKJPEGDATAIMAGEWRITER_H

#include <itkDataImageWriterBase.h>
#include <itkDataImageWriterPluginExport.h>

class ITKDATAIMAGEWRITERPLUGIN_EXPORT itkJPEGDataImageWriter: public itkDataImageWriterBase {
public:

    itkJPEGDataImageWriter();
    virtual ~itkJPEGDataImageWriter();

    virtual QString identifier()  const;
    virtual QString description() const;

    QStringList handled() const;

    static QStringList s_handled ();

    static bool registered();	

private:
    
    static const char ID[];
};

dtkAbstractDataWriter *createItkJPEGDataImageWriter();

#endif
