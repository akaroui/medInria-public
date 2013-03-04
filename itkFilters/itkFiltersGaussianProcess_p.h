#ifndef ITKFILTERSGAUSSIANPROCESS_P_H
#define ITKFILTERSGAUSSIANPROCESS_P_H

#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkSmartPointer.h>

#include "itkFiltersProcessBase_p.h"

#include <medMetaDataKeys.h>

#include "itkImage.h"
#include "itkCommand.h"
#include "itkSmoothingRecursiveGaussianImageFilter.h"

class itkFiltersGaussianProcess;

class itkFiltersGaussianProcessPrivate : public itkFiltersProcessBasePrivate
{
public:
    itkFiltersGaussianProcessPrivate(itkFiltersGaussianProcess *q = 0) : itkFiltersProcessBasePrivate(q) {}
    itkFiltersGaussianProcessPrivate(const itkFiltersGaussianProcessPrivate& other) : itkFiltersProcessBasePrivate(other) {}

    virtual ~itkFiltersGaussianProcessPrivate(void) {}
    
    double sigma;
    
    template <class PixelType> void update ( void )
    {
        this->setupFilter<PixelType>();
        this->setFilterDescription();
    }
    
    template <class PixelType> void setupFilter() 
    {
        typedef itk::Image< PixelType, 3 > ImageType;
        typedef itk::SmoothingRecursiveGaussianImageFilter< ImageType, ImageType >  GaussianFilterType;
        typename GaussianFilterType::Pointer gaussianFilter = GaussianFilterType::New();
    
        gaussianFilter->SetInput ( dynamic_cast<ImageType *> ( ( itk::Object* ) ( input->data() ) ) );
        gaussianFilter->SetSigma( sigma );
        
        callback = itk::CStyleCommand::New();
        callback->SetClientData ( ( void * ) this );
        callback->SetCallback ( itkFiltersGaussianProcessPrivate::eventCallback );
    
        gaussianFilter->AddObserver ( itk::ProgressEvent(), callback );
    
        gaussianFilter->Update();
        output->setData ( gaussianFilter->GetOutput() );
        
    }
    
    void setFilterDescription()
    {    
        QString newSeriesDescription = input->metadata ( medMetaDataKeys::SeriesDescription.key() );
        newSeriesDescription += " add filter (" + QString::number(sigma) + ")";
    
        output->addMetaData ( medMetaDataKeys::SeriesDescription.key(), newSeriesDescription );
    }
};

DTK_IMPLEMENT_PRIVATE(itkFiltersGaussianProcess, itkFiltersProcessBase)

#endif // ITKFILTERSGAUSSIANPROCESS_P_H
