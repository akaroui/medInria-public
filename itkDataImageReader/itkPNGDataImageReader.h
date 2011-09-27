#ifndef ITKPNGDATAIMAGEREADER_H
#define ITKPNGDATAIMAGEREADER_H

#include <itkDataImageReaderBase.h>
#include <itkDataImageReaderPluginExport.h>

class ITKDATAIMAGEREADERPLUGIN_EXPORT itkPNGDataImageReader: public itkDataImageReaderBase {
public:
    itkPNGDataImageReader();
    virtual ~itkPNGDataImageReader();

    virtual QString identifier()  const;
    virtual QString description() const;

    QStringList handled() const;

    static QStringList s_handled();

    static bool registered();	

private:

    static const char ID[];
};

dtkAbstractDataReader *createItkPNGDataImageReader();

#endif
