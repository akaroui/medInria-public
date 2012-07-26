//   Authors:	   Maxime Descoteaux, Jaime Garcia Guevara, Theodore Papadopoulo.
//
//   Description:  scale and orient glyph according to spherical harmonic representation   
//   vtkSphericalHarmonicGlyph is a filter that copies a SH representation (specified
//   as polygonal data) to every input point. 
//	
//   Copyright (c) 2007-2011, INRIA Sophia Antipolis, France, groups Odyssee, Athena.
//   Please see the copyright notice included in this distribution for full details.

#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkDataSet.h>
#include <vtkExecutive.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkTransform.h>

#include <vtkSphericalHarmonicGlyph.h>
//#include <vtkSphericalHarmonicSource.h>

vtkCxxRevisionMacro(vtkSphericalHarmonicGlyph,"$Revision: 0 $");
vtkStandardNewMacro(vtkSphericalHarmonicGlyph);

static void RGBToIndex(double R,double G,double B,double &index);

vtkSphericalHarmonicGlyph::vtkSphericalHarmonicGlyph()
{
  this->ScaleFactor = 1.0;
  this->ColorGlyphs = 1;
  this->ColorMode = COLOR_BY_SCALARS;
  this->SetNumberOfInputPorts(2);
  this->SphericalHarmonicSource = 0;
  this->TMatrix = 0;
}

vtkSphericalHarmonicGlyph::~vtkSphericalHarmonicGlyph()
{
  if(this->TMatrix)
    this->TMatrix->Delete();
}

int
vtkSphericalHarmonicGlyph::RequestData(vtkInformation*,vtkInformationVector** inputVector,
                                       vtkInformationVector* outputVector)
{
  // Get the info objects.
  vtkInformation* inInfo     = inputVector[0]->GetInformationObject(0);
  vtkInformation* sourceInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation* outInfo    = outputVector->GetInformationObject(0);

  const int numDirs = 1;

  // Get the input.
  vtkDataSet*  input     = vtkDataSet::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  const vtkIdType numPts = input->GetNumberOfPoints();  //number of points in the data

  if (numPts<1) {
    vtkErrorMacro(<<"No data to glyph!");
    return 1;
  }

  vtkDebugMacro(<<"Generating spherical harmonic glyphs");

  vtkPointData* pd        = input->GetPointData();
  vtkDataArray* inScalars = pd->GetScalars(GetSphericalHarmonicCoefficientsArrayName());
  vtkDataArray* inAniso   = pd->GetScalars(GetAnisotropyMeasureArrayName());

  // Allocate storage for output PolyData

  vtkPolyData* source    = vtkPolyData::SafeDownCast(sourceInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPoints*   sourcePts = source->GetPoints();  //shell
  // Number of points on the shell
  const vtkIdType numSourcePts   = sourcePts->GetNumberOfPoints();
  // Number of cells is the number of triangles in shell
  const vtkIdType numSourceCells = source->GetNumberOfCells();
  const unsigned newpts_sz = numDirs*numPts*numSourcePts;

  vtkPoints* newPts = vtkPoints::New();
  newPts->Allocate(newpts_sz);

  vtkPointData* newPointData = vtkPointData::New();
  newPointData->Allocate(newpts_sz);

  // Setting up for calls to PolyData::InsertNextCell()
  vtkPolyData*  output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPointData* outPD  = output->GetPointData();

  vtkCellArray* sourceCells;
  if ((sourceCells=source->GetVerts())->GetNumberOfCells()>0) {
    vtkCellArray* cells = vtkCellArray::New();
    cells->Allocate(numDirs*numPts*sourceCells->GetSize());
    output->SetVerts(cells);
    cells->Delete();
  }
  if ((sourceCells=this->GetSource()->GetLines())->GetNumberOfCells()>0) {
    vtkCellArray* cells = vtkCellArray::New();
    cells->Allocate(numDirs*numPts*sourceCells->GetSize());
    output->SetLines(cells);
    cells->Delete();
  }
  if ((sourceCells=this->GetSource()->GetPolys())->GetNumberOfCells()>0) {
    vtkCellArray* cells = vtkCellArray::New();
    cells->Allocate(numDirs*numPts*sourceCells->GetSize());
    output->SetPolys(cells);
    cells->Delete();
  }
  if ((sourceCells=this->GetSource()->GetStrips())->GetNumberOfCells()>0) {
    vtkCellArray* cells = vtkCellArray::New();
    cells->Allocate(numDirs*numPts*sourceCells->GetSize());
    output->SetStrips(cells);
    cells->Delete();
  }

  vtkFloatArray* newPointScalars = vtkFloatArray::New();
  newPointScalars->Allocate(numDirs*numPts*numSourcePts);

  // Only copy scalar data through.
  pd = this->GetSource()->GetPointData();

  vtkFloatArray* newScalars = 0;
  if (this->ColorGlyphs &&
      (this->ColorMode==COLOR_BY_DIRECTIONS ||
       (inAniso && this->ColorMode==COLOR_BY_SCALARS)))
  {
    newScalars = vtkFloatArray::New();
    newScalars->Allocate(numDirs*numPts*numSourcePts);
  } else {
    outPD->CopyAllOff();
    outPD->CopyScalarsOn();
    outPD->CopyAllocate(pd,numDirs*numPts*numSourcePts);
  }

  if (pd->GetNormals()) {
    vtkFloatArray* newNormals = vtkFloatArray::New();
    newNormals->SetNumberOfComponents(3);
    newNormals->Allocate(numDirs*3*numPts*numSourcePts);
    newNormals->Delete();
  }

  // First copy all topology (transformation independent)
  for (vtkIdType inPtId=0;inPtId<numPts;++inPtId) {
    const vtkIdType ptIncr = numDirs*inPtId*numSourcePts;
    for (vtkIdType cellId=0;cellId<numSourceCells;++cellId) {
      vtkCell*   cell    = this->GetSource()->GetCell(cellId);
      vtkIdList* cellPts = cell->GetPointIds();

      const int  npts    = cellPts->GetNumberOfIds();
      vtkIdType  pts[npts];

      for (int dir=0;dir<numDirs;++dir) {
        const vtkIdType subIncr = ptIncr+dir*numSourcePts;
        for (vtkIdType i=0;i<npts;++i)
          pts[i] = cellPts->GetId(i)+subIncr;
        output->InsertNextCell(cell->GetCellType(),npts,pts);
      }
    }
  }

  // Traverse all Input points, transforming glyph at Source points
  vtkTransform* trans = vtkTransform::New();
  trans->PreMultiply();

  // Visualization::vtkSphericalHarmonicSource* shs = Visualization::vtkSphericalHarmonicSource::New();
  //   shs->SetTesselation(3);

  for (vtkIdType inPtId=0;inPtId<numPts;++inPtId) {
    if (inPtId%10000==0) {
      this->UpdateProgress (static_cast<vtkFloatingPointType>(inPtId)/numPts);
      if (this->GetAbortExecute())
        break;
    }

    const vtkIdType ptIncr = numDirs*inPtId*numSourcePts;

    // Get the Spherical Harmonic vector.
    double sh[inScalars->GetNumberOfComponents()];
    inScalars->GetTuple(inPtId,sh);

    // Set harmonics and compute spherical function.
    this->SphericalHarmonicSource->SetSphericalHarmonics(sh);
    this->SphericalHarmonicSource->Update();

    vtkPointData* sourcePointData = this->SphericalHarmonicSource->GetOutput()->GetPointData();
    vtkPoints*    deformPts       = this->SphericalHarmonicSource->GetOutput()->GetPoints();

    trans->Identity();
    trans->SetMatrix(this->TMatrix);

    // translate Source to Input point
    double x[3];
    input->GetPoint(inPtId,x);
    trans->Translate(x[0],x[1],x[2]);

    trans->Scale(ScaleFactor,ScaleFactor,ScaleFactor);

    // Translate the deform pt to the correct x,y,z location
    trans->TransformPoints(deformPts,newPts);

    // This is for spherical values coloring
    // Need to set them at the right place in the list of all points
    for (int z=0;z<numSourcePts;++z)
      newPointScalars->InsertTuple(ptIncr+z,sourcePointData->GetScalars()->GetTuple(z));

    // Scalar color for anisotropy : one color per shell
    if (this->ColorGlyphs && inAniso && this->ColorMode==COLOR_BY_SCALARS) {
      const double s = inAniso->GetComponent(inPtId,0);
      for (vtkIdType i=0;i<numSourcePts;++i)
        newScalars->InsertTuple(ptIncr+i,&s);
    }

    // RGB color : color at every point of the spherical function
    if (this->ColorGlyphs && this->ColorMode==COLOR_BY_DIRECTIONS) {
      for (vtkIdType i=0;i<numSourcePts;++i) {
        double s,vect[3];
        this->SphericalHarmonicSource->GetOutput()->GetPoints()->GetPoint(i,vect);
        RGBToIndex(fabs(vect[0]),fabs(vect[1]),fabs(vect[2]),s);
        newScalars->InsertTuple(ptIncr+i,&s);
      }
    }
  }

  output->SetPoints(newPts);

  // Assigning color to PointData
  if (newScalars) {
    if(ColorMode==COLOR_BY_SCALARS) {
      cout << "Color by anisotropy...\n";
      newScalars->SetName(GetAnisotropyMeasureArrayName());
    } else {
      cout << "Color by RGB direction...\n";
      newScalars->SetName(GetRGBArrayName());
    }

    const int idx = outPD->AddArray(newScalars);
    outPD->SetActiveAttribute(idx,vtkDataSetAttributes::SCALARS);
    newScalars->Delete();
  }

  // Assigning spherical values
  if (newPointScalars) {
    newPointScalars->SetName(GetSphericalHarmonicValuesArrayName());
    //const int idx = outPD->AddArray(newPointScalars);
    //outPD->SetActiveAttribute(idx, vtkDataSetAttributes::SCALARS);
    //newPointScalars->PrintSelf(std::cout,0);
    newPointScalars->Delete();
  }

  newPts->Delete();
  newPointData->Delete();
  trans->Delete();

  return 1;
}

void
vtkSphericalHarmonicGlyph::SetSourceConnection(int id,vtkAlgorithmOutput* algOutput) {
  if (id<0) {
    vtkErrorMacro("Bad index " << id << " for source.");
    return;
  }

  const int numConnections = this->GetNumberOfInputConnections(1);
  if (id<numConnections)
    this->SetNthInputConnection(1, id, algOutput);
  else if (id==numConnections && algOutput)
    this->AddInputConnection(1, algOutput);
  else if (algOutput) {
    vtkWarningMacro("The source id provided is larger than the maximum source id, using "
                    << numConnections << " instead.");
    this->AddInputConnection(1, algOutput);
  }
}

vtkPolyData*
vtkSphericalHarmonicGlyph::GetSource() {
  if (this->GetNumberOfInputConnections(1)<1)
    return NULL;
  return vtkPolyData::SafeDownCast(this->GetExecutive()->GetInputData(1,0));
}

int
vtkSphericalHarmonicGlyph::FillInputPortInformation(int port,vtkInformation* info) {
  if (port==1) {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(),"vtkPolyData");
    return 1;
  }
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(),"vtkDataSet");
  return 1;
}

void
vtkSphericalHarmonicGlyph::PrintSelf(ostream& os,vtkIndent indent) {
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Source: " << this->GetSource() << "\n";
  os << indent << "Scale Factor: " << this->ScaleFactor << "\n";
  os << indent << "Color Glyphs: " << (this->ColorGlyphs ? "On\n" : "Off\n");
  os << indent << "Color Mode: " << this->ColorMode << endl;
}

// Function taken from 3D Slicer, SuperquadricTensorGlyph
//
// This is sort of the inverse of code from Gordon Kindlmann for mapping
// the mode (index value) to RGB. See vtkTensorMathematics for that code.
// There may be a simpler way to do this but this works.
// Note this expects a "0 1" Hue Range in the vtkLookupTable used to
// display the glyphs.

void RGBToIndex(double R,double G,double B,double &index) {

  // remove the gray part of the color.
  // this is so we can use the model where either R,G, or B is 0.
  // then we scale so that the max of the other two is one.

  double min = R;
  int minIdx = 0;

  if (G<min) {
    min = G;
    minIdx = 1;
  }
  if (B<min) {
    min = B;
    minIdx = 2;
  }

  // Make the smallest of R,G,B equal 0
  R -= min;
  G -= min;
  B -= min;

  // Now take the max, and scale it to be 1.
  double max = R;
  int maxIdx = 0;
  if (G>max) {
    max = G;
    maxIdx = 1;
  }
  if (B>max) {
    max = B;
    maxIdx = 2;
  }

  if (max!=0.0) {
    R /= max;
    G /= max;
    B /= max;
  }

  int sextant = 0;
  if (maxIdx==0 && minIdx==2) sextant = 0;
  if (maxIdx==1 && minIdx==2) sextant = 1;
  if (maxIdx==1 && minIdx==0) sextant = 2;
  if (maxIdx==2 && minIdx==0) sextant = 3;
  if (maxIdx==2 && minIdx==1) sextant = 4;
  if (maxIdx==0 && minIdx==1) sextant = 5;

  const double offset = 256/6;

  switch (sextant) {
  case 0: { index = G*offset;              break; }
  case 1: { index = offset+(1-R)*offset;   break; }
  case 2: { index = offset*2+B*offset;     break; }
  case 3: { index = offset*3+(1-G)*offset; break; }
  case 4: { index = offset*4+R*offset;     break; }
  case 5: { index = offset*5+(1-B)*offset; break; }
  }
}
