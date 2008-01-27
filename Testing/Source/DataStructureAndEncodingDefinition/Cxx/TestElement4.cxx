/*=========================================================================

  Program: GDCM (Grass Root DICOM). A DICOM library
  Module:  $URL$

  Copyright (c) 2006-2008 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include <iostream>

struct A
{
  float Internal;
};

struct B
{
  float Internal[1];
};

int TestElement4(int, char *[])
{
  std::cout << sizeof( A ) << std::endl;
  std::cout << sizeof( B ) << std::endl;

  return 0;
}