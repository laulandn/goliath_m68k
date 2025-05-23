// =================================================================================
//	UDragAndDropUtils.h				�1995-1998 Metrowerks Inc. All rights reserved.
// =================================================================================

#pragma once

#include <Drag.h>

class UDragAndDropUtils {
public:
	static Boolean	DroppedInTrash( DragReference inDragRef );
	static Boolean	CheckForOptionKey( DragReference inDragRef );
	static Boolean	CheckIfViewIsAlsoSender( DragReference inDragRef );
};
