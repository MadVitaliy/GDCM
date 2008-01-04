/*=========================================================================

  Program: GDCM (Grass Root DICOM). A DICOM library
  Module:  $URL$

  Copyright (c) 2006-2007 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "gdcmReader.h"
#include "gdcmImageReader.h"

#include "vtkImageData.h"
#include "vtkStructuredPointsWriter.h"

#include <pthread.h>

struct threadparams
{
  const char **filenames;
  unsigned int nfiles;
  char *scalarpointer;
};

void *ReadFilesThread(void *voidparams)
{
  threadparams *params = static_cast<threadparams *> (voidparams);

  const unsigned int nfiles = params->nfiles;
  for(unsigned int file = 0; file < nfiles; ++file)
    {
    /*
    // TODO: update progress
    pthread_mutex_lock(&params->lock);
    //section critique
    ReadingProgress+=params->stepProgress;
    pthread_mutex_unlock(&params->lock);
    */
    const char *filename = params->filenames[file];
    //std::cerr << filename << std::endl;

    gdcm::ImageReader reader;
    reader.SetFileName( filename );
    if( !reader.Read() )
      {
      return 0;
      }

    const gdcm::Image &image = reader.GetImage();
    unsigned long len = image.GetBufferLength();
    char *tempimage = new char[len];
    image.GetBuffer(tempimage);

    char * pointer = params->scalarpointer;
    memcpy(pointer + file*len, tempimage, len);
    delete[] tempimage;
    }

  return voidparams;
}

void ShowFilenames(const threadparams &params)
{
  std::cout << "start" << std::endl;
  for(unsigned int i = 0; i < params.nfiles; ++i)
    {
    const char *filename = params.filenames[i];
    std::cout << filename << std::endl;
    }
  std::cout << "end" << std::endl;
}

void ReadFiles(unsigned int nfiles, const char *filenames[])
{
  const char *reference= filenames[0]; // take the first image as reference

  gdcm::ImageReader reader;
  reader.SetFileName( reference );
  if( !reader.Read() )
    {
    // That would be very bad...
    abort();
    }

  const gdcm::Image &image = reader.GetImage();
  gdcm::PixelType pixeltype = image.GetPixelType();
  unsigned long len = image.GetBufferLength();
  const unsigned int *dims = image.GetDimensions();
  unsigned short pixelsize = pixeltype.GetPixelSize();

  vtkImageData *output = vtkImageData::New();
  output->SetDimensions(dims[0], dims[1], nfiles);

  switch( pixeltype )
    {
  case gdcm::PixelType::INT8:
    output->SetScalarType ( VTK_SIGNED_CHAR );
    break;
  case gdcm::PixelType::UINT8:
    output->SetScalarType ( VTK_UNSIGNED_CHAR );
    break;
  case gdcm::PixelType::INT16:
    output->SetScalarType ( VTK_SHORT );
    break;
  case gdcm::PixelType::UINT16:
    output->SetScalarType ( VTK_UNSIGNED_SHORT );
    break;
  default:
    abort();
    }

  output->SetNumberOfScalarComponents ( pixeltype.GetSamplesPerPixel() );
  if( image.GetPhotometricInterpretation() == 
    gdcm::PhotometricInterpretation::PALETTE_COLOR )
    {
    assert( output->GetNumberOfScalarComponents() == 1 );
    output->SetNumberOfScalarComponents ( 3 );
    }

  output->AllocateScalars();
  char * scalarpointer = static_cast<char*>(output->GetScalarPointer());

  const unsigned int nthreads = 4;
  threadparams params[nthreads];

  pthread_mutex_t lock;
  pthread_mutex_init(&lock, NULL);

  pthread_t *pthread = new pthread_t[nthreads];

  // There is nfiles, and nThreads
  assert( nfiles > nthreads );
  const unsigned int partition = nfiles / nthreads;
  //const unsigned int len = 2 * dims[0] * dims[1];
  for (unsigned int thread=0; thread < nthreads; ++thread)
    {
    params[thread].filenames = filenames + thread * partition;
    params[thread].nfiles = partition;
    assert( thread * partition < nfiles );
    params[thread].scalarpointer = scalarpointer + thread * partition * len;
    //assert( params[thread].scalarpointer < scalarpointer + 2 * dims[0] * dims[1] * dims[2] );
    // start thread:
    int res = pthread_create( &pthread[thread], NULL, ReadFilesThread, &params[thread]);
    if( res )
      {
      std::cerr << "Unable to start a new thread, pthread returned: " << res << std::endl;
      abort();
      }
    //ShowFilenames(params[thread]);
    }

  for (unsigned int thread=0;thread<nthreads;thread++)
    {
    pthread_join( pthread[thread], NULL);
    }
  delete[] pthread;

  pthread_mutex_destroy(&lock);
 
  // For some reason writing down the file is painfully slow...
  vtkStructuredPointsWriter *writer = vtkStructuredPointsWriter::New();
  writer->SetInput( output );
  writer->SetFileName( "/tmp/threadgdcm.vtk" );
  writer->SetFileTypeToBinary();
  writer->Write();
  writer->Delete();

  output->Print( std::cout );
  output->Delete();
}

int main(int argc, char *argv[])
{
  if( argc < 2 )
    {
    return 1;
    }

  const char **filenames = const_cast<const char**>(argv+1);
  const unsigned int nfiles = argc - 1;
  ReadFiles(nfiles, filenames);

  return 0;
}
