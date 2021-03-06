/* $TOG: validate.h /main/6 1998/02/09 14:30:17 kaleb $ */

/*

Copyright 1989, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
*/
/* $XFree86: xc/programs/Xserver/include/validate.h,v 1.3 1999/10/14 04:43:15 dawes Exp $ */

#ifndef VALIDATE_H
#define VALIDATE_H

#include "miscstruct.h"
#include "regionstr.h"

typedef enum { VTOther, VTStack, VTMove, VTUnmap, VTMap, VTBroken } VTKind;

/* union _Validate is now device dependent; see mivalidate.h for an example */
typedef union _Validate *ValidatePtr;

#define UnmapValData ((ValidatePtr)1)

#endif /* VALIDATE_H */
