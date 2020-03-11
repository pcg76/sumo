/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    GNEViewNetHelper.cpp
/// @author  Jakob Erdmann
/// @author  Pablo Alvarez Lopez
/// @date    Feb 2019
///
// A file used to reduce the size of GNEViewNet.h grouping structs and classes
/****************************************************************************/
#include <netedit/elements/additional/GNEPOI.h>
#include <netedit/elements/additional/GNEPoly.h>
#include <netedit/elements/additional/GNETAZ.h>
#include <netedit/elements/data/GNEDataSet.h>
#include <netedit/elements/data/GNEGenericData.h>
#include <netedit/elements/data/GNEEdgeData.h>
#include <netedit/elements/demand/GNEDemandElement.h>
#include <netedit/elements/network/GNEConnection.h>
#include <netedit/elements/network/GNECrossing.h>
#include <netedit/elements/network/GNEEdge.h>
#include <netedit/elements/network/GNEJunction.h>
#include <netedit/elements/network/GNELane.h>
#include <netedit/frames/common/GNESelectorFrame.h>
#include <netedit/frames/network/GNETLSEditorFrame.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/options/OptionsCont.h>

#include "GNEViewNetHelper.h"
#include "GNEViewNet.h"
#include "GNENet.h"
#include "GNEUndoList.h"
#include "GNEViewParent.h"
#include "GNEApplicationWindow.h"

// ===========================================================================
// member method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEViewNetHelper::ObjectsUnderCursor - methods
// ---------------------------------------------------------------------------

GNEViewNetHelper::ObjectsUnderCursor::ObjectsUnderCursor() {}


void
GNEViewNetHelper::ObjectsUnderCursor::updateObjectUnderCursor(const std::vector<GUIGlObject*>& GUIGlObjects, GNEPoly* editedPolyShape) {
    // first clear all containers
    myAttributeCarriers.clear();
    myNetworkElements.clear();
    myAdditionals.clear();
    myShapes.clear();
    myDemandElements.clear();
    myJunctions.clear();
    myEdges.clear();
    myLanes.clear();
    myCrossings.clear();
    myConnections.clear();
    myTAZs.clear();
    myPOIs.clear();
    myPolys.clear();
    myGenericDatas.clear();
    // set GUIGlObject
    sortGUIGlObjectsByAltitude(GUIGlObjects);
    // iterate over GUIGlObjects
    for (const auto& GUIGlObject : myGUIGlObjects) {
        // only continue if isn't GLO_NETWORKELEMENT (0)
        if (GUIGlObject->getType() != GLO_NETWORKELEMENT) {
            // cast attribute carrier from glObject
            myAttributeCarriers.push_back(dynamic_cast<GNEAttributeCarrier*>(GUIGlObject));
            // only continue if attributeCarrier isn't nullptr;
            if (myAttributeCarriers.back()) {
                // If we're editing a shape, ignore rest of elements (including other polygons)
                if (editedPolyShape != nullptr) {
                    if (myAttributeCarriers.back() == editedPolyShape) {
                        // cast Poly from attribute carrier
                        myPolys.push_back(dynamic_cast<GNEPoly*>(myAttributeCarriers.back()));
                    }
                } else {
                    // obtain tag property (only for improve code legibility)
                    const auto& tagValue = myAttributeCarriers.back()->getTagProperty();
                    // check if attributeCarrier can be casted into networkElement, additional or shape
                    if (tagValue.isNetworkElement()) {
                        // cast networkElement from attribute carrier
                        myNetworkElements.push_back(dynamic_cast<GNENetworkElement*>(myAttributeCarriers.back()));
                    } else if (tagValue.isAdditionalElement()) {
                        // cast additional element from attribute carrier
                        myAdditionals.push_back(dynamic_cast<GNEAdditional*>(myAttributeCarriers.back()));
                    } else if (tagValue.isTAZ()) {
                        // cast TAZ element from attribute carrier
                        myTAZs.push_back(dynamic_cast<GNETAZ*>(myAttributeCarriers.back()));
                    } else if (tagValue.isShape()) {
                        // cast shape element from attribute carrier
                        myShapes.push_back(dynamic_cast<GNEShape*>(myAttributeCarriers.back()));
                    } else if (tagValue.isDemandElement()) {
                        // cast demand element from attribute carrier
                        myDemandElements.push_back(dynamic_cast<GNEDemandElement*>(myAttributeCarriers.back()));
                    } else if (tagValue.isGenericData()) {
                        // cast generic data from attribute carrier
                        myGenericDatas.push_back(dynamic_cast<GNEEdgeData*>(myAttributeCarriers.back()));
                    }
                    // now set specify AC type
                    switch (GUIGlObject->getType()) {
                        case GLO_JUNCTION:
                            myJunctions.push_back(dynamic_cast<GNEJunction*>(myAttributeCarriers.back()));
                            break;
                        case GLO_EDGE: {
                            // fisrt obtain Edge
                            GNEEdge* edge = dynamic_cast<GNEEdge*>(myAttributeCarriers.back());
                            // check if parent edge is already inserted in myEdges (for example, due clicking over Geometry Points)
                            if (std::find(myEdges.begin(), myEdges.end(), edge) == myEdges.end()) {
                                myEdges.push_back(edge);
                            }
                            break;
                        }
                        case GLO_LANE: {
                            myLanes.push_back(dynamic_cast<GNELane*>(myAttributeCarriers.back()));
                            // check if edge's parent lane is already inserted in myEdges (for example, due clicking over Geometry Points)
                            if (std::find(myEdges.begin(), myEdges.end(), myLanes.back()->getParentEdge()) == myEdges.end()) {
                                myEdges.push_back(myLanes.back()->getParentEdge());
                            }
                            break;
                        }
                        case GLO_CROSSING:
                            myCrossings.push_back(dynamic_cast<GNECrossing*>(myAttributeCarriers.back()));
                            break;
                        case GLO_CONNECTION:
                            myConnections.push_back(dynamic_cast<GNEConnection*>(myAttributeCarriers.back()));
                            break;
                        case GLO_POI:
                            myPOIs.push_back(dynamic_cast<GNEPOI*>(myAttributeCarriers.back()));
                            break;
                        case GLO_POLYGON:
                            myPolys.push_back(dynamic_cast<GNEPoly*>(myAttributeCarriers.back()));
                            break;
                        case GLO_EDGEDATA:
                            myEdgeDatas.push_back(dynamic_cast<GNEEdgeData*>(myAttributeCarriers.back()));
                            break;
                        default:
                            break;
                    }
                }
            } else {
                myAttributeCarriers.pop_back();
            }
        }
    }
    /*
    // write information in debug mode (Currently disabled)
    WRITE_DEBUG("ObjectsUnderCursor: GUIGlObjects: " + toString(GUIGlObjects.size()) +
                ", AttributeCarriers: " + toString(myAttributeCarriers.size()) +
                ", NetworkElements: " + toString(myNetworkElements.size()) +
                ", Additionals: " + toString(myAdditionals.size()) +
                ", DemandElements: " + toString(myDemandElements.size()) +
                ", Shapes: " + toString(myShapes.size()) +
                ", Junctions: " + toString(myJunctions.size()) +
                ", Edges: " + toString(myEdges.size()) +
                ", Lanes: " + toString(myLanes.size()) +
                ", Crossings: " + toString(myCrossings.size()) +
                ", Connections: " + toString(myConnections.size()) +
                ", TAZs: " + toString(myTAZs.size()) +
                ", POIs: " + toString(myPOIs.size()) +
                ", Polys: " + toString(myPolys.size()));
    */
}


void
GNEViewNetHelper::ObjectsUnderCursor::swapLane2Edge() {
    // clear some containers
    myGUIGlObjects.clear();
    myAttributeCarriers.clear();
    myNetworkElements.clear();
    // fill containers using edges
    for (const auto& i : myEdges) {
        myGUIGlObjects.push_back(i);
        myAttributeCarriers.push_back(i);
        myNetworkElements.push_back(i);
    }
    // write information for debug
    WRITE_DEBUG("ObjectsUnderCursor: swapped Lanes to edges")
}


GUIGlID
GNEViewNetHelper::ObjectsUnderCursor::getGlIDFront() const {
    if (myGUIGlObjects.size() > 0) {
        return myGUIGlObjects.front()->getGlID();
    } else {
        return 0;
    }
}


GUIGlObjectType
GNEViewNetHelper::ObjectsUnderCursor::getGlTypeFront() const {
    if (myGUIGlObjects.size() > 0) {
        return myGUIGlObjects.front()->getType();
    } else {
        return GLO_NETWORK;
    }
}


GNEAttributeCarrier*
GNEViewNetHelper::ObjectsUnderCursor::getAttributeCarrierFront() const {
    if (myAttributeCarriers.size() > 0) {
        return myAttributeCarriers.front();
    } else {
        return nullptr;
    }
}


GNENetworkElement*
GNEViewNetHelper::ObjectsUnderCursor::getNetworkElementFront() const {
    if (myNetworkElements.size() > 0) {
        return myNetworkElements.front();
    } else {
        return nullptr;
    }
}


GNEAdditional*
GNEViewNetHelper::ObjectsUnderCursor::getAdditionalFront() const {
    if (myAdditionals.size() > 0) {
        return myAdditionals.front();
    } else {
        return nullptr;
    }
}


GNEShape*
GNEViewNetHelper::ObjectsUnderCursor::getShapeFront() const {
    if (myShapes.size() > 0) {
        return myShapes.front();
    } else {
        return nullptr;
    }
}


GNEDemandElement*
GNEViewNetHelper::ObjectsUnderCursor::getDemandElementFront() const {
    if (myDemandElements.size() > 0) {
        return myDemandElements.front();
    } else {
        return nullptr;
    }
}


GNEGenericData*
GNEViewNetHelper::ObjectsUnderCursor::getGenericDataElementFront() const {
    if (myGenericDatas.size() > 0) {
        return myGenericDatas.front();
    } else {
        return nullptr;
    }
}


GNEJunction*
GNEViewNetHelper::ObjectsUnderCursor::getJunctionFront() const {
    if (myJunctions.size() > 0) {
        return myJunctions.front();
    } else {
        return nullptr;
    }
}


GNEEdge*
GNEViewNetHelper::ObjectsUnderCursor::getEdgeFront() const {
    if (myEdges.size() > 0) {
        return myEdges.front();
    } else {
        return nullptr;
    }
}


GNELane*
GNEViewNetHelper::ObjectsUnderCursor::getLaneFront() const {
    if (myLanes.size() > 0) {
        return myLanes.front();
    } else {
        return nullptr;
    }
}


GNECrossing*
GNEViewNetHelper::ObjectsUnderCursor::getCrossingFront() const {
    if (myCrossings.size() > 0) {
        return myCrossings.front();
    } else {
        return nullptr;
    }
}


GNEConnection*
GNEViewNetHelper::ObjectsUnderCursor::getConnectionFront() const {
    if (myConnections.size() > 0) {
        return myConnections.front();
    } else {
        return nullptr;
    }
}


GNETAZ*
GNEViewNetHelper::ObjectsUnderCursor::getTAZFront() const {
    if (myTAZs.size() > 0) {
        return myTAZs.front();
    } else {
        return nullptr;
    }
}


GNEPOI*
GNEViewNetHelper::ObjectsUnderCursor::getPOIFront() const {
    if (myPOIs.size() > 0) {
        return myPOIs.front();
    } else {
        return nullptr;
    }
}


GNEPoly*
GNEViewNetHelper::ObjectsUnderCursor::getPolyFront() const {
    if (myPolys.size() > 0) {
        return myPolys.front();
    } else {
        return nullptr;
    }
}


GNEEdgeData*
GNEViewNetHelper::ObjectsUnderCursor::getEdgeDataElementFront() const {
    if (myEdgeDatas.size() > 0) {
        return myEdgeDatas.front();
    } else {
        return nullptr;
    }
}


const std::vector<GNEAttributeCarrier*>&
GNEViewNetHelper::ObjectsUnderCursor::getClickedAttributeCarriers() const {
    return myAttributeCarriers;
}


void
GNEViewNetHelper::ObjectsUnderCursor::sortGUIGlObjectsByAltitude(const std::vector<GUIGlObject*>& GUIGlObjects) {
    // first clear myGUIGlObjects
    myGUIGlObjects.clear();
    // declare a map to save sorted GUIGlObjects
    std::map<GUIGlObjectType, std::vector<GUIGlObject*> > mySortedGUIGlObjects;
    for (const auto& i : GUIGlObjects) {
        mySortedGUIGlObjects[i->getType()].push_back(i);
    }
    // move sorted GUIGlObjects into myGUIGlObjects using a reverse iterator
    for (std::map<GUIGlObjectType, std::vector<GUIGlObject*> >::reverse_iterator i = mySortedGUIGlObjects.rbegin(); i != mySortedGUIGlObjects.rend(); i++) {
        for (const auto& j : i->second) {
            myGUIGlObjects.push_back(j);
        }
    }
}

// ---------------------------------------------------------------------------
// GNEViewNetHelper::keyPressed - methods
// ---------------------------------------------------------------------------

GNEViewNetHelper::KeyPressed::KeyPressed() :
    myEventInfo(nullptr) {
}


void
GNEViewNetHelper::KeyPressed::update(void* eventData) {
    myEventInfo = (FXEvent*) eventData;
}


bool
GNEViewNetHelper::KeyPressed::shiftKeyPressed() const {
    if (myEventInfo) {
        return (myEventInfo->state & SHIFTMASK) != 0;
    } else {
        return false;
    }
}


bool
GNEViewNetHelper::KeyPressed::controlKeyPressed() const {
    if (myEventInfo) {
        return (myEventInfo->state & CONTROLMASK) != 0;
    } else {
        return false;
    }
}

// ---------------------------------------------------------------------------
// GNEViewNetHelper::MoveSingleElementValues - methods
// ---------------------------------------------------------------------------

GNEViewNetHelper::MoveSingleElementValues::MoveSingleElementValues(GNEViewNet* viewNet) :
    movingIndexShape(-1),
    myViewNet(viewNet),
    myMovingStartPos(false),
    myMovingEndPos(false),
    myJunctionToMove(nullptr),
    myEdgeToMove(nullptr),
    myPolyToMove(nullptr),
    myPOIToMove(nullptr),
    myAdditionalToMove(nullptr),
    myDemandElementToMove(nullptr),
    myTAZToMove(nullptr) {
}


bool
GNEViewNetHelper::MoveSingleElementValues::beginMoveSingleElementNetworkMode() {
    // first obtain moving reference (common for all)
    myRelativeClickedPosition = myViewNet->getPositionInformation();
    // check what type of AC will be moved
    if (myViewNet->myObjectsUnderCursor.getPolyFront()) {
        // calculate poly movement values (can be entire shape, single geometry points, altitude, etc.)
        return calculatePolyValues();
    } else if (myViewNet->myObjectsUnderCursor.getPOIFront()) {
        // set POI moved object
        myPOIToMove = myViewNet->myObjectsUnderCursor.getPOIFront();
        // Save original Position of POI in view
        originalPositionInView = myPOIToMove->getPositionInView();
        // there is moved items, then return true
        return true;
    } else if (myViewNet->myObjectsUnderCursor.getAdditionalFront()) {
        // set additionals moved object
        myAdditionalToMove = myViewNet->myObjectsUnderCursor.getAdditionalFront();
        // save current position of additional
        originalPositionInView = myAdditionalToMove->getPositionInView();
        // start additional geometry moving
        myAdditionalToMove->startGeometryMoving();
        // there is moved items, then return true
        return true;
    } else if (myViewNet->myObjectsUnderCursor.getTAZFront()) {
        // calculate TAZ movement values (can be entire shape or single geometry points)
        return calculateTAZValues();
    } else if (myViewNet->myObjectsUnderCursor.getJunctionFront()) {
        // set junction moved object
        myJunctionToMove = myViewNet->myObjectsUnderCursor.getJunctionFront();
        // Save original Position of Element in view
        originalPositionInView = myJunctionToMove->getPositionInView();
        // start junction geometry moving
        myJunctionToMove->startGeometryMoving();
        // there is moved items, then return true
        return true;
    } else if (myViewNet->myObjectsUnderCursor.getEdgeFront() || myViewNet->myObjectsUnderCursor.getLaneFront()) {
        // calculate Edge movement values (can be entire shape, single geometry points, altitude, etc.)
        return calculateEdgeValues();
    } else {
        // there isn't moved items, then return false
        return false;
    }
}


bool
GNEViewNetHelper::MoveSingleElementValues::beginMoveSingleElementDemandMode() {
    // first obtain moving reference (common for all)
    myRelativeClickedPosition = myViewNet->getPositionInformation();
    // check what type of AC will be moved
    if (myViewNet->myObjectsUnderCursor.getDemandElementFront()) {
        // set additionals moved object
        myDemandElementToMove = myViewNet->myObjectsUnderCursor.getDemandElementFront();
        // save current position of demand element
        originalPositionInView = myDemandElementToMove->getPositionInView();
        // start demand element geometry moving
        myDemandElementToMove->startGeometryMoving();
        // there is moved items, then return true
        return true;
    } else {
        // there isn't moved items, then return false
        return false;
    }
}


void
GNEViewNetHelper::MoveSingleElementValues::moveSingleElement() {
    // calculate offsetMovement depending of current mouse position and relative clicked position
    // @note  #3521: Add checkBox to allow moving elements... has to be implemented and used here
    Position offsetMovement = myViewNet->getPositionInformation() - myViewNet->myMoveSingleElementValues.myRelativeClickedPosition;
    // calculate Z depending of moveElevation
    if (myViewNet->myNetworkViewOptions.menuCheckMoveElevation->shown() && myViewNet->myNetworkViewOptions.menuCheckMoveElevation->getCheck() == TRUE) {
        // reset offset X and Y and use Y for Z
        offsetMovement = Position(0, 0, offsetMovement.y());
    } else {
        // leave z empty (because in this case offset only actuates over X-Y)
        offsetMovement.setz(0);
    }
    // check what element will be moved
    if (myPolyToMove) {
        // move shape's geometry without commiting changes depending if polygon is blocked
        if (myPolyToMove->isPolygonBlocked()) {
            // move entire shape
            myPolyToMove->moveEntireShape(originalShapeBeforeMoving, offsetMovement);
        } else {
            // move only a certain Geometry Point
            movingIndexShape = myPolyToMove->moveVertexShape(movingIndexShape, originalPositionInView, offsetMovement);
        }
    } else if (myPOIToMove) {
        // Move POI's geometry without commiting changes
        myPOIToMove->moveGeometry(originalPositionInView, offsetMovement);
    } else if (myJunctionToMove) {
        // Move Junction's geometry without commiting changes
        myJunctionToMove->moveGeometry(originalPositionInView, offsetMovement);
    } else if (myEdgeToMove) {
        // check if we're moving the start or end position, or a geometry point
        if (myMovingStartPos) {
            myEdgeToMove->moveShapeStart(originalPositionInView, offsetMovement);
        } else if (myMovingEndPos) {
            myEdgeToMove->moveShapeEnd(originalPositionInView, offsetMovement);
        } else {
            // move edge's geometry without commiting changes
            movingIndexShape = myEdgeToMove->moveVertexShape(movingIndexShape, originalPositionInView, offsetMovement);
        }
    } else if (myAdditionalToMove && (myAdditionalToMove->isAdditionalBlocked() == false)) {
        // Move Additional geometry without commiting changes
        myAdditionalToMove->moveGeometry(offsetMovement);
    } else if (myDemandElementToMove/* && (myDemandElementToMove->isDemandElementBlocked() == false)*/) {
        // Move DemandElement geometry without commiting changes
        myDemandElementToMove->moveGeometry(offsetMovement);
    } else if (myTAZToMove) {
        /// move TAZ's geometry without commiting changes depending if polygon is blocked
        if (myTAZToMove->isShapeBlocked()) {
            // move entire shape
            myTAZToMove->moveEntireShape(originalShapeBeforeMoving, offsetMovement);
        } else {
            // move only a certain Geometry Point
            movingIndexShape = myTAZToMove->moveVertexShape(movingIndexShape, originalPositionInView, offsetMovement);
        }
    }
    // update view (needed to see the movement)
    myViewNet->update();
}


void
GNEViewNetHelper::MoveSingleElementValues::finishMoveSingleElement() {
    if (myPolyToMove) {
        myPolyToMove->commitShapeChange(originalShapeBeforeMoving, myViewNet->getUndoList());
        myPolyToMove = nullptr;
    } else if (myPOIToMove) {
        myPOIToMove->commitGeometryMoving(originalPositionInView, myViewNet->getUndoList());
        myPOIToMove = nullptr;
    } else if (myJunctionToMove) {
        // check if in the moved position there is another Junction and it will be merged
        if (!myViewNet->mergeJunctions(myJunctionToMove, originalPositionInView)) {
            myJunctionToMove->commitGeometryMoving(originalPositionInView, myViewNet->getUndoList());
        }
        myJunctionToMove = nullptr;
    } else if (myEdgeToMove) {
        // commit change depending of what was moved
        if (myMovingStartPos) {
            myEdgeToMove->commitShapeStartChange(originalPositionInView, myViewNet->getUndoList());
            myMovingStartPos = false;
        } else if (myMovingEndPos) {
            myEdgeToMove->commitShapeEndChange(originalPositionInView, myViewNet->getUndoList());
            myMovingEndPos = false;
        } else {
            myEdgeToMove->commitShapeChange(originalShapeBeforeMoving, myViewNet->getUndoList());
        }
        myEdgeToMove = nullptr;
    } else if (myAdditionalToMove) {
        myAdditionalToMove->commitGeometryMoving(myViewNet->getUndoList());
        myAdditionalToMove->endGeometryMoving();
        myAdditionalToMove = nullptr;
    } else if (myDemandElementToMove) {
        myDemandElementToMove->commitGeometryMoving(myViewNet->getUndoList());
        myDemandElementToMove->endGeometryMoving();
        myDemandElementToMove = nullptr;
    } else if (myTAZToMove) {
        myTAZToMove->commitShapeChange(originalShapeBeforeMoving, myViewNet->getUndoList());
        myTAZToMove = nullptr;
    }
}


bool
GNEViewNetHelper::MoveSingleElementValues::calculatePolyValues() {
    // set Poly to move
    myPolyToMove = myViewNet->myObjectsUnderCursor.getPolyFront();
    // now we have two cases: if we're editing the X-Y coordenade or the altitude (z)
    if (myViewNet->myNetworkViewOptions.menuCheckMoveElevation->shown() && myViewNet->myNetworkViewOptions.menuCheckMoveElevation->getCheck() == TRUE) {
        // check if in the clicked position a geometry point exist
        int existentIndex = myPolyToMove->getVertexIndex(myViewNet->getPositionInformation(), false, false);
        if (existentIndex != -1) {
            // save original shape (needed for commit change)
            myViewNet->myMoveSingleElementValues.originalShapeBeforeMoving = myPolyToMove->getShape();
            // obtain existent index
            myViewNet->myMoveSingleElementValues.movingIndexShape = existentIndex;
            myViewNet->myMoveSingleElementValues.originalPositionInView = myPolyToMove->getShape()[existentIndex];
            // poly values sucesfully calculated, then return true
            return true;
        } else {
            // stop poly moving
            myPolyToMove = nullptr;
            // poly values wasn't calculated, then return false
            return false;
        }
    } else {
        // save original shape (needed for commit change)
        myViewNet->myMoveSingleElementValues.originalShapeBeforeMoving = myPolyToMove->getShape();
        // save clicked position as moving original position
        myViewNet->myMoveSingleElementValues.originalPositionInView = myViewNet->getPositionInformation();
        // obtain index of vertex to move if shape isn't blocked
        if ((myPolyToMove->isPolygonBlocked() == false) && (myPolyToMove->isMovementBlocked() == false)) {
            // check if we want to remove a Geometry Point
            if (myViewNet->myKeyPressed.shiftKeyPressed()) {
                // check if we're clicked over a Geometry Point
                myViewNet->myMoveSingleElementValues.movingIndexShape = myPolyToMove->getVertexIndex(myViewNet->myMoveSingleElementValues.originalPositionInView, false, false);
                if (myViewNet->myMoveSingleElementValues.movingIndexShape != -1) {
                    myPolyToMove->deleteGeometryPoint(myViewNet->myMoveSingleElementValues.originalPositionInView);
                    // after removing Geomtery Point, reset PolyToMove
                    myPolyToMove = nullptr;
                    // poly values wasn't calculated, then return false
                    return false;
                }
                // poly values sucesfully calculated, then return true
                return true;
            } else {
                // obtain index of vertex to move and moving reference
                myViewNet->myMoveSingleElementValues.movingIndexShape = myPolyToMove->getVertexIndex(myViewNet->myMoveSingleElementValues.originalPositionInView, false, false);
                // check if a new Vertex must be created
                if (myViewNet->myMoveSingleElementValues.movingIndexShape == -1) {
                    if (myPolyToMove->getShape().distance2D(myViewNet->myMoveSingleElementValues.originalPositionInView) <= 0.8) {
                        // create new geometry point
                        myViewNet->myMoveSingleElementValues.movingIndexShape = myPolyToMove->getVertexIndex(myViewNet->myMoveSingleElementValues.originalPositionInView, true, true);
                    } else {
                        // nothing to move, then return false
                        return false;
                    }
                }
                // set Z value
                myViewNet->myMoveSingleElementValues.originalPositionInView.setz(myPolyToMove->getShape()[myViewNet->myMoveSingleElementValues.movingIndexShape].z());
                // poly values sucesfully calculated, then return true
                return true;
            }
        } else {
            myViewNet->myMoveSingleElementValues.movingIndexShape = -1;
            // check if polygon has the entire movement blocked, or only the shape blocked
            return (myPolyToMove->isMovementBlocked() == false);
        }
    }
}


bool
GNEViewNetHelper::MoveSingleElementValues::calculateEdgeValues() {
    if (myViewNet->myKeyPressed.shiftKeyPressed()) {
        // edit end point
        myViewNet->myObjectsUnderCursor.getEdgeFront()->editEndpoint(myViewNet->getPositionInformation(), myViewNet->myUndoList);
        // edge values wasn't calculated, then return false
        return false;
    } else {
        // assign clicked edge to edgeToMove
        myEdgeToMove = myViewNet->myObjectsUnderCursor.getEdgeFront();
        // check if we clicked over a start or end position
        if (myEdgeToMove->clickedOverShapeStart(myViewNet->getPositionInformation())) {
            // save start pos
            myViewNet->myMoveSingleElementValues.originalPositionInView = myEdgeToMove->getNBEdge()->getGeometry().front();
            myViewNet->myMoveSingleElementValues.myMovingStartPos = true;
            // start geometry moving
            myEdgeToMove->startGeometryMoving();
            // edge values sucesfully calculated, then return true
            return true;
        } else if (myEdgeToMove->clickedOverShapeEnd(myViewNet->getPositionInformation())) {
            // save end pos
            myViewNet->myMoveSingleElementValues.originalPositionInView = myEdgeToMove->getNBEdge()->getGeometry().back();
            myViewNet->myMoveSingleElementValues.myMovingEndPos = true;
            // start geometry moving
            myEdgeToMove->startGeometryMoving();
            // edge values sucesfully calculated, then return true
            return true;
        } else {
            // now we have two cases: if we're editing the X-Y coordenade or the altitude (z)
            if (myViewNet->myNetworkViewOptions.menuCheckMoveElevation->shown() && myViewNet->myNetworkViewOptions.menuCheckMoveElevation->getCheck() == TRUE) {
                // check if in the clicked position a geometry point exist
                int existentIndex = myEdgeToMove->getVertexIndex(myViewNet->getPositionInformation(), false, false);
                if (existentIndex != -1) {
                    myViewNet->myMoveSingleElementValues.movingIndexShape = existentIndex;
                    myViewNet->myMoveSingleElementValues.originalPositionInView = myEdgeToMove->getNBEdge()->getInnerGeometry()[existentIndex];
                    // start geometry moving
                    myEdgeToMove->startGeometryMoving();
                    // edge values sucesfully calculated, then return true
                    return true;
                } else {
                    // stop edge moving
                    myEdgeToMove = nullptr;
                    // edge values wasn't calculated, then return false
                    return false;
                }
            } else {
                // save original shape (needed for commit change)
                myViewNet->myMoveSingleElementValues.originalShapeBeforeMoving = myEdgeToMove->getNBEdge()->getInnerGeometry();
                // obtain index of vertex to move and moving reference
                myViewNet->myMoveSingleElementValues.movingIndexShape = myEdgeToMove->getVertexIndex(myViewNet->getPositionInformation(), false, false);
                // if index doesn't exist, create it snapping new edge to grid
                if (myViewNet->myMoveSingleElementValues.movingIndexShape == -1) {
                    myViewNet->myMoveSingleElementValues.movingIndexShape = myEdgeToMove->getVertexIndex(myViewNet->getPositionInformation(), true, true);
                }
                // make sure that myViewNet->myMoveSingleElementValues.movingIndexShape isn't -1
                if (myViewNet->myMoveSingleElementValues.movingIndexShape != -1) {
                    myViewNet->myMoveSingleElementValues.originalPositionInView = myEdgeToMove->getNBEdge()->getInnerGeometry()[myViewNet->myMoveSingleElementValues.movingIndexShape];
                    // start geometry moving
                    myEdgeToMove->startGeometryMoving();
                    // edge values sucesfully calculated, then return true
                    return true;
                } else {
                    // edge values wasn't calculated, then return false
                    return false;
                }
            }
        }
    }
}


bool
GNEViewNetHelper::MoveSingleElementValues::calculateTAZValues() {
    // set TAZ to move
    myTAZToMove = myViewNet->myObjectsUnderCursor.getTAZFront();
    // save original shape (needed for commit change)
    myViewNet->myMoveSingleElementValues.originalShapeBeforeMoving = myTAZToMove->getTAZShape();
    // save clicked position as moving original position
    myViewNet->myMoveSingleElementValues.originalPositionInView = myViewNet->getPositionInformation();
    // obtain index of vertex to move if shape isn't blocked
    if ((myTAZToMove->isShapeBlocked() == false) && (myTAZToMove->isAdditionalBlocked() == false)) {
        // check if we want to remove a Geometry Point
        if (myViewNet->myKeyPressed.shiftKeyPressed()) {
            // check if we're clicked over a Geometry Point
            myViewNet->myMoveSingleElementValues.movingIndexShape = myTAZToMove->getVertexIndex(myViewNet->myMoveSingleElementValues.originalPositionInView, false, false);
            if (myViewNet->myMoveSingleElementValues.movingIndexShape != -1) {
                myTAZToMove->deleteGeometryPoint(myViewNet->myMoveSingleElementValues.originalPositionInView);
                // after removing Geomtery Point, reset TAZToMove
                myTAZToMove = nullptr;
                // TAZ values wasn't calculated, then return false
                return false;
            }
            // TAZ values sucesfully calculated, then return true
            return true;
        } else {
            // obtain index of vertex to move and moving reference
            myViewNet->myMoveSingleElementValues.movingIndexShape = myTAZToMove->getVertexIndex(myViewNet->myMoveSingleElementValues.originalPositionInView, false, false);
            if (myViewNet->myMoveSingleElementValues.movingIndexShape == -1) {
                // create new geometry point
                myViewNet->myMoveSingleElementValues.movingIndexShape = myTAZToMove->getVertexIndex(myViewNet->myMoveSingleElementValues.originalPositionInView, true, true);
            }
            // TAZ values sucesfully calculated, then return true
            return true;
        }
    } else {
        // abort moving index shape
        myViewNet->myMoveSingleElementValues.movingIndexShape = -1;
        // check if TAZ has the entire movement blocked, or only the shape blocked
        return (myTAZToMove->isAdditionalBlocked() == false);
    }
}

// ---------------------------------------------------------------------------
// GNEViewNetHelper::MoveMultipleElementValues - methods
// ---------------------------------------------------------------------------

GNEViewNetHelper::MoveMultipleElementValues::MoveMultipleElementValues(GNEViewNet* viewNet) :
    myViewNet(viewNet),
    myMovingSelection(false) {
}


void
GNEViewNetHelper::MoveMultipleElementValues::beginMoveSelection(GNEAttributeCarrier* originAC) {
    // enable moving selection
    myMovingSelection = true;
    // save clicked position (to calculate offset)
    myClickedPosition = myViewNet->getPositionInformation();
    // obtain Junctions and edges selected
    std::vector<GNEJunction*> selectedJunctions = myViewNet->getNet()->retrieveJunctions(true);
    std::vector<GNEEdge*> selectedEdges = myViewNet->getNet()->retrieveEdges(true);
    // Junctions are always moved, then save position of current selected junctions (Needed when mouse is released)
    for (auto i : selectedJunctions) {
        // save junction position
        myMovedJunctionOriginPositions[i] = i->getPositionInView();
        // start geometry moving
        i->startGeometryMoving();
    }
    // make special movement depending of clicked AC
    if (originAC->getTagProperty().getTag() == SUMO_TAG_JUNCTION) {
        // if clicked element is a junction, move shapes of all selected edges
        for (auto i : selectedEdges) {
            // save entire edge geometry
            myMovedEdgesOriginShape[i] = i->getNBEdge()->getInnerGeometry();
            // start geometry moving
            i->startGeometryMoving();
        }
    } else if (originAC->getTagProperty().getTag() == SUMO_TAG_EDGE) {
        // obtain clicked edge
        GNEEdge* clickedEdge = dynamic_cast<GNEEdge*>(originAC);
        // if clicked edge has origin and destiny junction selected, move shapes of all selected edges
        if (myMovedJunctionOriginPositions.count(clickedEdge->getGNEJunctionSource()) > 0 &&
                myMovedJunctionOriginPositions.count(clickedEdge->getGNEJunctionDestiny()) > 0) {
            for (auto i : selectedEdges) {
                // save entire edge geometry
                myMovedEdgesOriginShape[i] = i->getNBEdge()->getInnerGeometry();
                // start geometry moving
                i->startGeometryMoving();
            }
        } else {
            // declare three groups for dividing edges
            std::vector<GNEEdge*> noJunctionsSelected;
            std::vector<GNEEdge*> originJunctionSelected;
            std::vector<GNEEdge*> destinyJunctionSelected;
            // divide selected edges into four groups, depending of the selection of their junctions
            for (auto i : selectedEdges) {
                bool originSelected = myMovedJunctionOriginPositions.count(i->getGNEJunctionSource()) > 0;
                bool destinySelected = myMovedJunctionOriginPositions.count(i->getGNEJunctionDestiny()) > 0;
                // bot junctions selected
                if (!originSelected && !destinySelected) {
                    noJunctionsSelected.push_back(i);
                } else if (originSelected && !destinySelected) {
                    originJunctionSelected.push_back(i);
                } else if (!originSelected && destinySelected) {
                    destinyJunctionSelected.push_back(i);
                } else if (!originSelected && !destinySelected) {
                    // save edge geometry
                    myMovedEdgesOriginShape[i] = i->getNBEdge()->getInnerGeometry();
                    // start geometry moving
                    i->startGeometryMoving();
                }
            }
            // save original shape of all noJunctionsSelected edges (needed for commit change)
            for (auto i : noJunctionsSelected) {
                myMovedEgdesGeometryPoints[i] = new MoveSingleElementValues(myViewNet);
                // save edge geometry
                myMovedEgdesGeometryPoints[i]->originalShapeBeforeMoving = i->getNBEdge()->getInnerGeometry();
                // start geometry moving
                i->startGeometryMoving();
            }
            // obtain index shape of clicked edge
            int index = clickedEdge->getVertexIndex(myViewNet->getPositionInformation(), true, true);
            // check that index is valid
            if (index < 0) {
                // end geometry moving without changes in moved junctions
                for (auto i : myMovedJunctionOriginPositions) {
                    i.first->endGeometryMoving();
                }
                // end geometry moving without changes in moved edges
                for (auto i : myMovedEdgesOriginShape) {
                    i.first->endGeometryMoving();
                }
                // end geometry moving without changes in moved shapes
                for (auto i : myMovedEgdesGeometryPoints) {
                    i.first->endGeometryMoving();
                }
                // stop moving selection
                myMovingSelection = false;
                // clear containers
                myMovedJunctionOriginPositions.clear();
                myMovedEdgesOriginShape.clear();
                // delete all movedEgdesGeometryPoints before clear container
                for (const auto& i : myMovedEgdesGeometryPoints) {
                    delete i.second;
                }
                myMovedEgdesGeometryPoints.clear();
            } else {
                // save index and original position
                myMovedEgdesGeometryPoints[clickedEdge] = new MoveSingleElementValues(myViewNet);
                myMovedEgdesGeometryPoints[clickedEdge]->movingIndexShape = index;
                myMovedEgdesGeometryPoints[clickedEdge]->originalPositionInView = myViewNet->getPositionInformation();
                // start moving of clicked edge AFTER getting vertex Index
                clickedEdge->startGeometryMoving();
                // do the same for  the rest of noJunctionsSelected edges
                for (auto i : noJunctionsSelected) {
                    if (i != clickedEdge) {
                        myMovedEgdesGeometryPoints[i] = new MoveSingleElementValues(myViewNet);
                        // save index and original position
                        myMovedEgdesGeometryPoints[i]->movingIndexShape = i->getVertexIndex(myViewNet->getPositionInformation(), true, true);
                        // set originalPosition depending if edge is opposite to clicked edge
                        if (i->getOppositeEdge() == clickedEdge) {
                            myMovedEgdesGeometryPoints[i]->originalPositionInView = myViewNet->getPositionInformation();
                        } else {
                            myMovedEgdesGeometryPoints[i]->originalPositionInView = i->getNBEdge()->getInnerGeometry()[myMovedEgdesGeometryPoints[i]->movingIndexShape];
                        }
                        // start moving of clicked edge AFTER getting vertex Index
                        i->startGeometryMoving();
                    }
                }
            }
        }
    }
}


void
GNEViewNetHelper::MoveMultipleElementValues::moveSelection() {
    // calculate offset between current position and original position
    Position offsetMovement = myViewNet->getPositionInformation() - myClickedPosition;
    // calculate Z depending of Grid
    if (myViewNet->myNetworkViewOptions.menuCheckMoveElevation->shown() && myViewNet->myNetworkViewOptions.menuCheckMoveElevation->getCheck() == TRUE) {
        // reset offset X and Y and use Y for Z
        offsetMovement = Position(0, 0, offsetMovement.y());
    } else {
        // leave z empty (because in this case offset only actuates over X-Y)
        offsetMovement.setz(0);
    }
    // move selected junctions
    for (auto i : myMovedJunctionOriginPositions) {
        i.first->moveGeometry(i.second, offsetMovement);
    }
    // move entire edge shapes
    for (auto i : myMovedEdgesOriginShape) {
        i.first->moveEntireShape(i.second, offsetMovement);
    }
    // move partial shapes
    for (auto i : myMovedEgdesGeometryPoints) {
        i.first->moveVertexShape(i.second->movingIndexShape, i.second->originalPositionInView, offsetMovement);
    }
    // update view (needed to see the movement)
    myViewNet->update();
}


void
GNEViewNetHelper::MoveMultipleElementValues::finishMoveSelection() {
    // begin undo list
    myViewNet->getUndoList()->p_begin("position of selected elements");
    // commit positions of moved junctions
    for (auto i : myMovedJunctionOriginPositions) {
        i.first->commitGeometryMoving(i.second, myViewNet->getUndoList());
    }
    // commit shapes of entired moved edges
    for (auto i : myMovedEdgesOriginShape) {
        i.first->commitShapeChange(i.second, myViewNet->getUndoList());
    }
    //commit shapes of partial moved shapes
    for (auto i : myMovedEgdesGeometryPoints) {
        i.first->commitShapeChange(i.second->originalShapeBeforeMoving, myViewNet->getUndoList());
    }
    // end undo list
    myViewNet->getUndoList()->p_end();
    // stop moving selection
    myMovingSelection = false;
    // clear containers
    myMovedJunctionOriginPositions.clear();
    myMovedEdgesOriginShape.clear();
    // delete all movedEgdesGeometryPoints before clear container
    for (const auto& i : myMovedEgdesGeometryPoints) {
        delete i.second;
    }
    myMovedEgdesGeometryPoints.clear();
}


bool
GNEViewNetHelper::MoveMultipleElementValues::isMovingSelection() const {
    return myMovingSelection;
}

// ---------------------------------------------------------------------------
// GNEViewNetHelper::VehicleOptions - methods
// ---------------------------------------------------------------------------

GNEViewNetHelper::VehicleOptions::VehicleOptions(GNEViewNet* viewNet) :
    myViewNet(viewNet) {
}


void
GNEViewNetHelper::VehicleOptions::buildVehicleOptionsMenuChecks() {
    UNUSED_PARAMETER(myViewNet);
    // currently unused
}


void
GNEViewNetHelper::VehicleOptions::hideVehicleOptionsMenuChecks() {
    // currently unused
}

// ---------------------------------------------------------------------------
// GNEViewNetHelper::VehicleTypeOptions - methods
// ---------------------------------------------------------------------------

GNEViewNetHelper::VehicleTypeOptions::VehicleTypeOptions(GNEViewNet* viewNet) :
    myViewNet(viewNet) {
}


void
GNEViewNetHelper::VehicleTypeOptions::buildVehicleTypeOptionsMenuChecks() {
    UNUSED_PARAMETER(myViewNet);
    // currently unused
}


void
GNEViewNetHelper::VehicleTypeOptions::hideVehicleTypeOptionsMenuChecks() {
    // currently unused
}

// ---------------------------------------------------------------------------
// GNEViewNetHelper::SelectingArea - methods
// ---------------------------------------------------------------------------

GNEViewNetHelper::SelectingArea::SelectingArea(GNEViewNet* viewNet) :
    selectingUsingRectangle(false),
    startDrawing(false),
    myViewNet(viewNet) {
}


void
GNEViewNetHelper::SelectingArea::beginRectangleSelection() {
    selectingUsingRectangle = true;
    selectionCorner1 = myViewNet->getPositionInformation();
    selectionCorner2 = selectionCorner1;
}


void
GNEViewNetHelper::SelectingArea::moveRectangleSelection() {
    // start drawing
    startDrawing = true;
    // only update selection corner 2
    selectionCorner2 = myViewNet->getPositionInformation();
    // update status bar
    myViewNet->setStatusBarText("Selection width:" + toString(fabs(selectionCorner1.x() - selectionCorner2.x()))
                                + " height:" + toString(fabs(selectionCorner1.y() - selectionCorner2.y()))
                                + " diagonal:" + toString(selectionCorner1.distanceTo2D(selectionCorner2)));
    // update view (needed to update rectangle)
    myViewNet->update();
}


void
GNEViewNetHelper::SelectingArea::finishRectangleSelection() {
    // finish rectangle selection
    selectingUsingRectangle = false;
    startDrawing = false;
}


void
GNEViewNetHelper::SelectingArea::processRectangleSelection() {
    // shift held down on mouse-down and mouse-up and check that rectangle exist
    if ((abs(selectionCorner1.x() - selectionCorner2.x()) > 0.01) &&
            (abs(selectionCorner1.y() - selectionCorner2.y()) > 0.01) &&
            myViewNet->myKeyPressed.shiftKeyPressed()) {
        // create boundary between two corners
        Boundary rectangleBoundary;
        rectangleBoundary.add(selectionCorner1);
        rectangleBoundary.add(selectionCorner2);
        // process selection within boundary
        processBoundarySelection(rectangleBoundary);
    }
}


std::vector<GNEEdge*>
GNEViewNetHelper::SelectingArea::processEdgeRectangleSelection() {
    // declare vector for selection
    std::vector<GNEEdge*> result;
    // shift held down on mouse-down and mouse-up and check that rectangle exist
    if ((abs(selectionCorner1.x() - selectionCorner2.x()) > 0.01) &&
            (abs(selectionCorner1.y() - selectionCorner2.y()) > 0.01) &&
            myViewNet->myKeyPressed.shiftKeyPressed()) {
        // create boundary between two corners
        Boundary rectangleBoundary;
        rectangleBoundary.add(selectionCorner1);
        rectangleBoundary.add(selectionCorner2);
        if (myViewNet->makeCurrent()) {
            // obtain all ACs in Rectangle BOundary
            std::set<std::pair<std::string, GNEAttributeCarrier*> > ACsInBoundary = myViewNet->getAttributeCarriersInBoundary(rectangleBoundary);
            // Filter ACs in Boundary and get only edges
            for (auto i : ACsInBoundary) {
                if (i.second->getTagProperty().getTag() == SUMO_TAG_EDGE) {
                    result.push_back(dynamic_cast<GNEEdge*>(i.second));
                }
            }
            myViewNet->makeNonCurrent();
        }
    }
    return result;
}


void
GNEViewNetHelper::SelectingArea::processShapeSelection(const PositionVector& shape) {
    processBoundarySelection(shape.getBoxBoundary());
}


void
GNEViewNetHelper::SelectingArea::drawRectangleSelection(const RGBColor& color) const {
    if (selectingUsingRectangle) {
        glPushMatrix();
        glTranslated(0, 0, GLO_MAX - 1);
        GLHelper::setColor(color);
        glLineWidth(2);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glBegin(GL_QUADS);
        glVertex2d(selectionCorner1.x(), selectionCorner1.y());
        glVertex2d(selectionCorner1.x(), selectionCorner2.y());
        glVertex2d(selectionCorner2.x(), selectionCorner2.y());
        glVertex2d(selectionCorner2.x(), selectionCorner1.y());
        glEnd();
        glPopMatrix();
    }
}


void
GNEViewNetHelper::SelectingArea::processBoundarySelection(const Boundary& boundary) {
    if (myViewNet->makeCurrent()) {
        std::set<std::pair<std::string, GNEAttributeCarrier*> > ACsInBoundary = myViewNet->getAttributeCarriersInBoundary(boundary);
        // filter ACsInBoundary depending of current supermode
        std::set<std::pair<std::string, GNEAttributeCarrier*> > ACsInBoundaryFiltered;
        for (const auto& i : ACsInBoundary) {
            if (((myViewNet->myEditModes.currentSupermode == Supermode::NETWORK) && !i.second->getTagProperty().isDemandElement()) ||
                    ((myViewNet->myEditModes.currentSupermode == Supermode::DEMAND) && i.second->getTagProperty().isDemandElement())) {
                ACsInBoundaryFiltered.insert(i);
            }
        }
        // declare two sets of attribute carriers, one for select and another for unselect
        std::vector<GNEAttributeCarrier*> ACToSelect;
        std::vector<GNEAttributeCarrier*> ACToUnselect;
        // reserve memory (we assume that in the worst case we're going to insert all elements of ACsInBoundaryFiltered
        ACToSelect.reserve(ACsInBoundaryFiltered.size());
        ACToUnselect.reserve(ACsInBoundaryFiltered.size());
        // in restrict AND replace mode all current selected attribute carriers will be unselected
        if ((myViewNet->myViewParent->getSelectorFrame()->getModificationModeModul()->getModificationMode() == GNESelectorFrame::ModificationMode::Operation::RESTRICT) ||
                (myViewNet->myViewParent->getSelectorFrame()->getModificationModeModul()->getModificationMode() == GNESelectorFrame::ModificationMode::Operation::REPLACE)) {
            // obtain selected ACs depending of current supermode
            std::vector<GNEAttributeCarrier*> selectedAC = myViewNet->getNet()->getSelectedAttributeCarriers(false);
            // add id into ACs to unselect
            for (auto i : selectedAC) {
                ACToUnselect.push_back(i);
            }
        }
        // iterate over AtributeCarriers obtained of boundary an place it in ACToSelect or ACToUnselect
        for (auto i : ACsInBoundaryFiltered) {
            switch (myViewNet->myViewParent->getSelectorFrame()->getModificationModeModul()->getModificationMode()) {
                case GNESelectorFrame::ModificationMode::Operation::SUB:
                    ACToUnselect.push_back(i.second);
                    break;
                case GNESelectorFrame::ModificationMode::Operation::RESTRICT:
                    if (std::find(ACToUnselect.begin(), ACToUnselect.end(), i.second) != ACToUnselect.end()) {
                        ACToSelect.push_back(i.second);
                    }
                    break;
                default:
                    ACToSelect.push_back(i.second);
                    break;
            }
        }
        // select junctions and their connections and crossings if Auto select junctions is enabled (note: only for "add mode")
        if (myViewNet->autoSelectNodes() && (myViewNet->myViewParent->getSelectorFrame()->getModificationModeModul()->getModificationMode() == GNESelectorFrame::ModificationMode::Operation::ADD)) {
            std::vector<GNEEdge*> edgesToSelect;
            // iterate over ACToSelect and extract edges
            for (auto i : ACToSelect) {
                if (i->getTagProperty().getTag() == SUMO_TAG_EDGE) {
                    edgesToSelect.push_back(dynamic_cast<GNEEdge*>(i));
                }
            }
            // iterate over extracted edges
            for (auto i : edgesToSelect) {
                // select junction source and all their connections and crossings
                ACToSelect.push_back(i->getGNEJunctionSource());
                for (auto j : i->getGNEJunctionSource()->getGNEConnections()) {
                    ACToSelect.push_back(j);
                }
                for (auto j : i->getGNEJunctionSource()->getGNECrossings()) {
                    ACToSelect.push_back(j);
                }
                // select junction destiny and all their connections crossings
                ACToSelect.push_back(i->getGNEJunctionDestiny());
                for (auto j : i->getGNEJunctionDestiny()->getGNEConnections()) {
                    ACToSelect.push_back(j);
                }
                for (auto j : i->getGNEJunctionDestiny()->getGNECrossings()) {
                    ACToSelect.push_back(j);
                }
            }
        }
        // only continue if there is ACs to select or unselect
        if ((ACToSelect.size() + ACToUnselect.size()) > 0) {
            // first unselect AC of ACToUnselect and then selects AC of ACToSelect
            myViewNet->myUndoList->p_begin("selection using rectangle");
            for (auto i : ACToUnselect) {
                i->setAttribute(GNE_ATTR_SELECTED, "0", myViewNet->myUndoList);
            }
            for (auto i : ACToSelect) {
                if (i->getTagProperty().isSelectable()) {
                    i->setAttribute(GNE_ATTR_SELECTED, "1", myViewNet->myUndoList);
                }
            }
            myViewNet->myUndoList->p_end();
        }
        myViewNet->makeNonCurrent();
    }
}

// ---------------------------------------------------------------------------
// GNEViewNetHelper::TestingMode - methods
// ---------------------------------------------------------------------------

GNEViewNetHelper::TestingMode::TestingMode(GNEViewNet* viewNet) :
    myViewNet(viewNet),
    myTestingEnabled(OptionsCont::getOptions().getBool("gui-testing")),
    myTestingWidth(0),
    myTestingHeight(0) {
}


void
GNEViewNetHelper::TestingMode::initTestingMode() {
    // first check if testing mode is enabled and window size is correct
    if (myTestingEnabled && OptionsCont::getOptions().isSet("window-size")) {
        std::vector<std::string> windowSize = OptionsCont::getOptions().getStringVector("window-size");
        // make sure that given windows size has exactly two valid int values
        if ((windowSize.size() == 2) && GNEAttributeCarrier::canParse<int>(windowSize[0]) && GNEAttributeCarrier::canParse<int>(windowSize[1])) {
            myTestingWidth = GNEAttributeCarrier::parse<int>(windowSize[0]);
            myTestingHeight = GNEAttributeCarrier::parse<int>(windowSize[1]);
        } else {
            WRITE_ERROR("Invalid windows size-format: " + toString(windowSize) + "for option 'window-size'");
        }
    }
}


void
GNEViewNetHelper::TestingMode::drawTestingElements(GUIMainWindow* mainWindow) {
    // first check if testing mode is neabled
    if (myTestingEnabled) {
        // check if main windows has to be resized
        if (myTestingWidth > 0 && ((myViewNet->getWidth() != myTestingWidth) || (myViewNet->getHeight() != myTestingHeight))) {
            // only resize once to avoid flickering
            //std::cout << " before resize: view=" << getWidth() << ", " << getHeight() << " app=" << mainWindow->getWidth() << ", " << mainWindow->getHeight() << "\n";
            mainWindow->resize(myTestingWidth + myTestingWidth - myViewNet->getWidth(), myTestingHeight + myTestingHeight - myViewNet->getHeight());
            //std::cout << " directly after resize: view=" << getWidth() << ", " << getHeight() << " app=" << mainWindow->getWidth() << ", " << mainWindow->getHeight() << "\n";
            myTestingWidth = 0;
        }
        //std::cout << " fixed: view=" << getWidth() << ", " << getHeight() << " app=" << mainWindow->getWidth() << ", " << mainWindow->getHeight() << "\n";
        // draw pink square in the upper left corner on top of everything
        glPushMatrix();
        const double size = myViewNet->p2m(32);
        Position center = myViewNet->screenPos2NetPos(8, 8);
        GLHelper::setColor(RGBColor::MAGENTA);
        glTranslated(center.x(), center.y(), GLO_MAX - 1);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBegin(GL_QUADS);
        glVertex2d(0, 0);
        glVertex2d(0, -size);
        glVertex2d(size, -size);
        glVertex2d(size, 0);
        glEnd();
        glPopMatrix();
        glPushMatrix();
        // show box with the current position relative to pink square
        Position posRelative = myViewNet->screenPos2NetPos(myViewNet->getWidth() - 40, myViewNet->getHeight() - 20);
        // adjust cursor position (24,25) to show exactly the same position as in function netedit.leftClick(match, X, Y)
        GLHelper::drawTextBox(toString(myViewNet->getWindowCursorPosition().x() - 24) + " " + toString(myViewNet->getWindowCursorPosition().y() - 25), posRelative, GLO_MAX - 1, myViewNet->p2m(20), RGBColor::BLACK, RGBColor::WHITE);
        glPopMatrix();
    }
}


bool
GNEViewNetHelper::TestingMode::isTestingEnabled() const {
    return myTestingEnabled;
}

// ---------------------------------------------------------------------------
// GNEViewNetHelper::SaveElements - methods
// ---------------------------------------------------------------------------

GNEViewNetHelper::SaveElements::SaveElements(GNEViewNet* viewNet) :
    saveNetwork(nullptr),
    saveAdditionalElements(nullptr),
    saveDemandElements(nullptr),
    saveDataElements(nullptr),
    myViewNet(viewNet) {
}


void
GNEViewNetHelper::SaveElements::buildSaveElementsButtons() {
    // create save network button
    saveNetwork = new FXButton(myViewNet->getViewParent()->getGNEAppWindows()->getToolbarsGrip().saveElements,
                               "\tSave network\tSave network.", GUIIconSubSys::getIcon(GUIIcon::SAVENETWORKELEMENTS),
                               myViewNet->getViewParent()->getGNEAppWindows(), MID_HOTKEY_CTRL_S_STOPSIMULATION_SAVENETWORK, GUIDesignButtonToolbar);
    saveNetwork->create();
    // create save additional elements button
    saveAdditionalElements = new FXButton(myViewNet->getViewParent()->getGNEAppWindows()->getToolbarsGrip().saveElements,
                                          "\tSave additional elements\tSave additional elements.", GUIIconSubSys::getIcon(GUIIcon::SAVEADDITIONALELEMENTS),
                                          myViewNet->getViewParent()->getGNEAppWindows(), MID_HOTKEY_CTRL_SHIFT_A_SAVEADDITIONALS, GUIDesignButtonToolbar);
    saveAdditionalElements->create();
    // create save demand elements button
    saveDemandElements = new FXButton(myViewNet->getViewParent()->getGNEAppWindows()->getToolbarsGrip().saveElements,
                                      "\tSave demand elements\tSave demand elements.", GUIIconSubSys::getIcon(GUIIcon::SAVEDEMANDELEMENTS),
                                      myViewNet->getViewParent()->getGNEAppWindows(), MID_HOTKEY_CTRL_SHIFT_D_SAVEDEMANDELEMENTS, GUIDesignButtonToolbar);
    saveDemandElements->create();
    // create save data elements button
    saveDataElements = new FXButton(myViewNet->getViewParent()->getGNEAppWindows()->getToolbarsGrip().saveElements,
                                    "\tSave data elements\tSave data elements.", GUIIconSubSys::getIcon(GUIIcon::SAVEDATAELEMENTS),
                                    myViewNet->getViewParent()->getGNEAppWindows(), MID_HOTKEY_CTRL_SHIFT_B_SAVEDATAELEMENTS, GUIDesignButtonToolbar);
    saveDataElements->create();
    // recalc menu bar because there is new elements
    myViewNet->getViewParent()->getGNEAppWindows()->getToolbarsGrip().saveElements->recalc();
    // show menu bar modes
    myViewNet->getViewParent()->getGNEAppWindows()->getToolbarsGrip().saveElements->show();
}

// ---------------------------------------------------------------------------
// GNEViewNetHelper::EditModes - methods
// ---------------------------------------------------------------------------

GNEViewNetHelper::EditModes::EditModes(GNEViewNet* viewNet) :
    currentSupermode(Supermode::NONE),
    networkEditMode(NetworkEditMode::NETWORK_INSPECT),
    demandEditMode(DemandEditMode::DEMAND_INSPECT),
    dataEditMode(DataEditMode::DATA_INSPECT),
    networkButton(nullptr),
    demandButton(nullptr),
    dataButton(nullptr),
    myViewNet(viewNet) {
}


void
GNEViewNetHelper::EditModes::buildSuperModeButtons() {
    // create network button
    networkButton = new MFXCheckableButton(false, myViewNet->getViewParent()->getGNEAppWindows()->getToolbarsGrip().superModes, "Network\t\tSet mode for edit network elements.",
                                           GUIIconSubSys::getIcon(GUIIcon::SUPERMODENETWORK), myViewNet, MID_HOTKEY_F2_SUPERMODE_NETWORK, GUIDesignButtonToolbarSupermode);
    networkButton->create();
    // create demand button
    demandButton = new MFXCheckableButton(false, myViewNet->getViewParent()->getGNEAppWindows()->getToolbarsGrip().superModes, "Demand\t\tSet mode for edit traffic demand.",
                                          GUIIconSubSys::getIcon(GUIIcon::SUPERMODEDEMAND), myViewNet, MID_HOTKEY_F3_SUPERMODE_DEMAND, GUIDesignButtonToolbarSupermode);
    demandButton->create();
    // create data button
    dataButton = new MFXCheckableButton(false, myViewNet->getViewParent()->getGNEAppWindows()->getToolbarsGrip().superModes, "Data\t\tSet mode for edit data demand.",
                                        GUIIconSubSys::getIcon(GUIIcon::SUPERMODEDATA), myViewNet, MID_HOTKEY_F4_SUPERMODE_DATA, GUIDesignButtonToolbarSupermode);
    dataButton->create();
    // recalc menu bar because there is new elements
    myViewNet->getViewParent()->getGNEAppWindows()->getToolbarsGrip().modes->recalc();
    // show menu bar modes
    myViewNet->getViewParent()->getGNEAppWindows()->getToolbarsGrip().modes->show();
}


void
GNEViewNetHelper::EditModes::setSupermode(Supermode supermode) {
    if (supermode == currentSupermode) {
        myViewNet->setStatusBarText("Mode already selected");
        if (myViewNet->myCurrentFrame != nullptr) {
            myViewNet->myCurrentFrame->focusUpperElement();
        }
    } else {
        myViewNet->setStatusBarText("");
        // abort current operation
        myViewNet->abortOperation(false);
        // set super mode
        currentSupermode = supermode;
        // set supermodes
        if (supermode == Supermode::NETWORK) {
            // change buttons
            networkButton->setChecked(true);
            demandButton->setChecked(false);
            dataButton->setChecked(false);
            // show network buttons
            myViewNet->myNetworkCheckableButtons.showNetworkCheckableButtons();
            // hide demand buttons
            myViewNet->myDemandCheckableButtons.hideDemandCheckableButtons();
            // hide data buttons
            myViewNet->myDataCheckableButtons.hideDataCheckableButtons();
            // force update network mode
            setNetworkEditMode(networkEditMode, true);
        } else if (supermode == Supermode::DEMAND) {
            // change buttons
            networkButton->setChecked(false);
            demandButton->setChecked(true);
            dataButton->setChecked(false);
            // hide network buttons
            myViewNet->myNetworkCheckableButtons.hideNetworkCheckableButtons();
            // show demand buttons
            myViewNet->myDemandCheckableButtons.showDemandCheckableButtons();
            // hide data buttons
            myViewNet->myDataCheckableButtons.hideDataCheckableButtons();
            // force update demand mode
            setDemandEditMode(demandEditMode, true);
        } else if (supermode == Supermode::DATA) {
            // change buttons
            networkButton->setChecked(false);
            demandButton->setChecked(false);
            dataButton->setChecked(true);
            // hide network buttons
            myViewNet->myNetworkCheckableButtons.hideNetworkCheckableButtons();
            // hide demand buttons
            myViewNet->myDemandCheckableButtons.hideDemandCheckableButtons();
            // show data buttons
            myViewNet->myDataCheckableButtons.showDataCheckableButtons();
            // force update data mode
            setDataEditMode(dataEditMode, true);
        }
        // update buttons
        networkButton->update();
        demandButton->update();
        dataButton->update();
        // update Supermode CommandButtons in GNEAppWindows
        myViewNet->myViewParent->getGNEAppWindows()->updateSuperModeMenuCommands(currentSupermode);
    }
}


void
GNEViewNetHelper::EditModes::setNetworkEditMode(NetworkEditMode mode, bool force) {
    if ((mode == networkEditMode) && !force) {
        myViewNet->setStatusBarText("Network mode already selected");
        if (myViewNet->myCurrentFrame != nullptr) {
            myViewNet->myCurrentFrame->focusUpperElement();
        }
    } else if (networkEditMode == NetworkEditMode::NETWORK_TLS && !myViewNet->myViewParent->getTLSEditorFrame()->isTLSSaved()) {
        myViewNet->setStatusBarText("save modifications in TLS before change mode");
        myViewNet->myCurrentFrame->focusUpperElement();
    } else {
        myViewNet->setStatusBarText("");
        myViewNet->abortOperation(false);
        // stop editing of custom shapes
        myViewNet->myEditShapes.stopEditCustomShape();
        // set new Network mode
        networkEditMode = mode;
        // for common modes (Inspect/Delete/Select/move) change also the other supermode
        if (networkEditMode == NetworkEditMode::NETWORK_INSPECT) {
            demandEditMode = DemandEditMode::DEMAND_INSPECT;
            dataEditMode = DataEditMode::DATA_INSPECT;
        } else if (networkEditMode == NetworkEditMode::NETWORK_DELETE) {
            demandEditMode = DemandEditMode::DEMAND_DELETE;
            dataEditMode = DataEditMode::DATA_DELETE;
        } else if (networkEditMode == NetworkEditMode::NETWORK_SELECT) {
            demandEditMode = DemandEditMode::DEMAND_SELECT;
            dataEditMode = DataEditMode::DATA_SELECT;
        } else if (networkEditMode == NetworkEditMode::NETWORK_MOVE) {
            demandEditMode = DemandEditMode::DEMAND_MOVE;
        }
        // certain modes require a recomputing
        switch (mode) {
            case NetworkEditMode::NETWORK_CONNECT:
            case NetworkEditMode::NETWORK_PROHIBITION:
            case NetworkEditMode::NETWORK_TLS:
                // modes which depend on computed data
                myViewNet->myNet->computeNetwork(myViewNet->myViewParent->getGNEAppWindows());
                break;
            default:
                break;
        }
        // update network mode specific controls
        myViewNet->updateNetworkModeSpecificControls();
    }
}


void
GNEViewNetHelper::EditModes::setDemandEditMode(DemandEditMode mode, bool force) {
    if ((mode == demandEditMode) && !force) {
        myViewNet->setStatusBarText("Demand mode already selected");
        if (myViewNet->myCurrentFrame != nullptr) {
            myViewNet->myCurrentFrame->focusUpperElement();
        }
    } else {
        myViewNet->setStatusBarText("");
        myViewNet->abortOperation(false);
        // stop editing of custom shapes
        myViewNet->myEditShapes.stopEditCustomShape();
        // set new Demand mode
        demandEditMode = mode;
        // for common modes (Inspect/Delete/Select/Move) change also the other supermode
        if (demandEditMode == DemandEditMode::DEMAND_INSPECT) {
            networkEditMode = NetworkEditMode::NETWORK_INSPECT;
            dataEditMode = DataEditMode::DATA_INSPECT;
        } else if (demandEditMode == DemandEditMode::DEMAND_DELETE) {
            networkEditMode = NetworkEditMode::NETWORK_DELETE;
            dataEditMode = DataEditMode::DATA_DELETE;
        } else if (demandEditMode == DemandEditMode::DEMAND_SELECT) {
            networkEditMode = NetworkEditMode::NETWORK_SELECT;
            dataEditMode = DataEditMode::DATA_SELECT;
        } else if (demandEditMode == DemandEditMode::DEMAND_MOVE) {
            networkEditMode = NetworkEditMode::NETWORK_MOVE;
        }
        // demand modes require ALWAYS a recomputing
        myViewNet->myNet->computeNetwork(myViewNet->myViewParent->getGNEAppWindows());
        // update DijkstraRouter of RouteCalculatorInstance
        GNEDemandElement::getRouteCalculatorInstance()->updateDijkstraRouter();
        // update network mode specific controls
        myViewNet->updateDemandModeSpecificControls();
    }
}


void
GNEViewNetHelper::EditModes::setDataEditMode(DataEditMode mode, bool force) {
    if ((mode == dataEditMode) && !force) {
        myViewNet->setStatusBarText("Data mode already selected");
        if (myViewNet->myCurrentFrame != nullptr) {
            myViewNet->myCurrentFrame->focusUpperElement();
        }
    } else {
        myViewNet->setStatusBarText("");
        myViewNet->abortOperation(false);
        // stop editing of custom shapes
        myViewNet->myEditShapes.stopEditCustomShape();
        // set new Data mode
        dataEditMode = mode;
        // for common modes (Inspect/Delete/Select/Move) change also the other supermode
        if (dataEditMode == DataEditMode::DATA_INSPECT) {
            networkEditMode = NetworkEditMode::NETWORK_INSPECT;
            demandEditMode = DemandEditMode::DEMAND_INSPECT;
        } else if (dataEditMode == DataEditMode::DATA_DELETE) {
            networkEditMode = NetworkEditMode::NETWORK_DELETE;
            demandEditMode = DemandEditMode::DEMAND_DELETE;
        } else if (dataEditMode == DataEditMode::DATA_SELECT) {
            networkEditMode = NetworkEditMode::NETWORK_SELECT;
            demandEditMode = DemandEditMode::DEMAND_SELECT;
        }
        // data modes require ALWAYS a recomputing
        myViewNet->myNet->computeNetwork(myViewNet->myViewParent->getGNEAppWindows());
        // update network mode specific controls
        myViewNet->updateDataModeSpecificControls();
    }
}

// ---------------------------------------------------------------------------
// GNEViewNetHelper::CommonViewOptions - methods
// ---------------------------------------------------------------------------


GNEViewNetHelper::CommonViewOptions::CommonViewOptions(GNEViewNet* viewNet) :
    menuCheckShowGrid(nullptr),
    menuCheckDrawSpreadVehicles(nullptr),
    myViewNet(viewNet) {
}


void
GNEViewNetHelper::CommonViewOptions::buildCommonViewOptionsMenuChecks() {
    menuCheckShowGrid = new FXMenuCheck(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions,
        ("Grid\t\tshow grid and restrict movement to the grid (size defined in visualization options)"),
        myViewNet, MID_GNE_COMMONVIEWOPTIONS_SHOWGRID, LAYOUT_FIX_HEIGHT);
    menuCheckShowGrid->setHeight(23);
    menuCheckShowGrid->setCheck(false);
    menuCheckShowGrid->create();

    menuCheckDrawSpreadVehicles = new FXMenuCheck(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions,
        ("Spread vehicles\t\tDraw vehicles spread in lane or in depart position"),
        myViewNet, MID_GNE_COMMONVIEWOPTIONS_DRAWSPREADVEHICLES, LAYOUT_FIX_HEIGHT);
    menuCheckDrawSpreadVehicles->setHeight(23);
    menuCheckDrawSpreadVehicles->setCheck(false);
    menuCheckDrawSpreadVehicles->create();

    // always recalc after creating new elements
    myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions->recalc();
}


void
GNEViewNetHelper::CommonViewOptions::getVisibleCommonMenuCommands(std::vector<FXMenuCheck*>& commands) const {
    // save visible menu commands in commands vector
    if (menuCheckShowGrid->shown()) {
        commands.push_back(menuCheckShowGrid);
    }
    if (menuCheckDrawSpreadVehicles->shown()) {
        commands.push_back(menuCheckDrawSpreadVehicles);
    }
}


bool
GNEViewNetHelper::CommonViewOptions::drawSpreadVehicles() const {
    return (menuCheckDrawSpreadVehicles->getCheck() == TRUE);
}

// ---------------------------------------------------------------------------
// GNEViewNetHelper::NetworkViewOptions - methods
// ---------------------------------------------------------------------------

GNEViewNetHelper::NetworkViewOptions::NetworkViewOptions(GNEViewNet* viewNet) :
    menuCheckShowDemandElements(nullptr),
    menuCheckSelectEdges(nullptr),
    menuCheckShowConnections(nullptr),
    menuCheckHideConnections(nullptr),
    menuCheckExtendSelection(nullptr),
    menuCheckChangeAllPhases(nullptr),
    menuCheckWarnAboutMerge(nullptr),
    menuCheckShowJunctionBubble(nullptr),
    menuCheckMoveElevation(nullptr),
    menuCheckChainEdges(nullptr),
    menuCheckAutoOppositeEdge(nullptr),
    myViewNet(viewNet) {
}


void
GNEViewNetHelper::NetworkViewOptions::buildNetworkViewOptionsMenuChecks() {
    menuCheckShowDemandElements = new FXMenuCheck(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions,
        ("Show demand elements\t\tToggle show demand elements"),
        myViewNet, MID_GNE_NETWORKVIEWOPTIONS_SHOWDEMANDELEMENTS, LAYOUT_FIX_HEIGHT);
    menuCheckShowDemandElements->setHeight(23);
    menuCheckShowDemandElements->setCheck(false);
    menuCheckShowDemandElements->create();

    menuCheckSelectEdges = new FXMenuCheck(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions,
        ("Select edges\t\tToggle whether clicking should select " + toString(SUMO_TAG_EDGE) + "s or " + toString(SUMO_TAG_LANE) + "s").c_str(),
        myViewNet, MID_GNE_NETWORKVIEWOPTIONS_SELECTEDGES, LAYOUT_FIX_HEIGHT);
    menuCheckSelectEdges->setHeight(23);
    menuCheckSelectEdges->setCheck(true);
    menuCheckSelectEdges->create();

    menuCheckShowConnections = new FXMenuCheck(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions,
        ("Show " + toString(SUMO_TAG_CONNECTION) + "s\t\tToggle show " + toString(SUMO_TAG_CONNECTION) + "s over " + toString(SUMO_TAG_JUNCTION) + "s").c_str(),
        myViewNet, MID_GNE_NETWORKVIEWOPTIONS_SHOWCONNECTIONS, LAYOUT_FIX_HEIGHT);
    menuCheckShowConnections->setHeight(23);
    menuCheckShowConnections->setCheck(myViewNet->getVisualisationSettings().showLane2Lane);
    menuCheckShowConnections->create();

    menuCheckHideConnections = new FXMenuCheck(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions,
        ("hide " + toString(SUMO_TAG_CONNECTION) + "s\t\tHide connections").c_str(),
        myViewNet, MID_GNE_NETWORKVIEWOPTIONS_HIDECONNECTIONS, LAYOUT_FIX_HEIGHT);
    menuCheckHideConnections->setHeight(23);
    menuCheckHideConnections->setCheck(false);
    menuCheckHideConnections->create();

    menuCheckExtendSelection = new FXMenuCheck(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions,
        ("Auto-select " + toString(SUMO_TAG_JUNCTION) + "s\t\tToggle whether selecting multiple " + toString(SUMO_TAG_EDGE) + "s should automatically select their " + toString(SUMO_TAG_JUNCTION) + "s").c_str(),
        myViewNet, MID_GNE_NETWORKVIEWOPTIONS_EXTENDSELECTION, LAYOUT_FIX_HEIGHT);
    menuCheckExtendSelection->setHeight(23);
    menuCheckExtendSelection->setCheck(false);
    menuCheckExtendSelection->create();

    menuCheckChangeAllPhases = new FXMenuCheck(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions,
        ("Apply change to all phases\t\tToggle whether clicking should apply state changes to all phases of the current " + toString(SUMO_TAG_TRAFFIC_LIGHT) + " plan").c_str(),
        myViewNet, MID_GNE_NETWORKVIEWOPTIONS_CHANGEALLPHASES, LAYOUT_FIX_HEIGHT);
    menuCheckChangeAllPhases->setHeight(23);
    menuCheckChangeAllPhases->setCheck(false);
    menuCheckChangeAllPhases->create();

    menuCheckWarnAboutMerge = new FXMenuCheck(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions,
        ("Ask for merge\t\tAsk for confirmation before merging " + toString(SUMO_TAG_JUNCTION) + ".").c_str(),
        myViewNet, MID_GNE_NETWORKVIEWOPTIONS_ASKFORMERGE, LAYOUT_FIX_HEIGHT);
    menuCheckWarnAboutMerge->setHeight(23);
    menuCheckWarnAboutMerge->setCheck(true);
    menuCheckWarnAboutMerge->create();

    menuCheckShowJunctionBubble = new FXMenuCheck(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions,
        ("Bubbles\t\tShow bubbles over " + toString(SUMO_TAG_JUNCTION) + "'s shapes.").c_str(),
        myViewNet, MID_GNE_NETWORKVIEWOPTIONS_SHOWBUBBLES, LAYOUT_FIX_HEIGHT);
    menuCheckShowJunctionBubble->setHeight(23);
    menuCheckShowJunctionBubble->setCheck(false);
    menuCheckShowJunctionBubble->create();

    menuCheckMoveElevation = new FXMenuCheck(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions,
        ("Elevation\t\tApply mouse movement to elevation instead of x,y position"),
        myViewNet, MID_GNE_NETWORKVIEWOPTIONS_MOVEELEVATION, LAYOUT_FIX_HEIGHT);
    menuCheckMoveElevation->setHeight(23);
    menuCheckMoveElevation->setCheck(false);
    menuCheckMoveElevation->create();

    menuCheckChainEdges = new FXMenuCheck(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions,
        ("Chain\t\tCreate consecutive " + toString(SUMO_TAG_EDGE) + "s with a single click (hit ESC to cancel chain).").c_str(),
        myViewNet, MID_GNE_NETWORKVIEWOPTIONS_CHAINEDGES, LAYOUT_FIX_HEIGHT);
    menuCheckChainEdges->setHeight(23);
    menuCheckChainEdges->setCheck(false);
    menuCheckChainEdges->create();

    menuCheckAutoOppositeEdge = new FXMenuCheck(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions,
        ("Two-way\t\tAutomatically create an " + toString(SUMO_TAG_EDGE) + " in the opposite direction").c_str(),
        myViewNet, MID_GNE_NETWORKVIEWOPTIONS_AUTOOPPOSITEEDGES, LAYOUT_FIX_HEIGHT);
    menuCheckAutoOppositeEdge->setHeight(23);
    menuCheckAutoOppositeEdge->setCheck(false);
    menuCheckAutoOppositeEdge->create();

    // always recalc after creating new elements
    myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions->recalc();
}


void
GNEViewNetHelper::NetworkViewOptions::hideNetworkViewOptionsMenuChecks() {
    menuCheckShowDemandElements->hide();
    menuCheckSelectEdges->hide();
    menuCheckShowConnections->hide();
    menuCheckHideConnections->hide();
    menuCheckExtendSelection->hide();
    menuCheckChangeAllPhases->hide();
    menuCheckWarnAboutMerge->hide();
    menuCheckShowJunctionBubble->hide();
    menuCheckMoveElevation->hide();
    menuCheckChainEdges->hide();
    menuCheckAutoOppositeEdge->hide();
    // Also hide toolbar grip
    myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions->show();
}


void
GNEViewNetHelper::NetworkViewOptions::getVisibleNetworkMenuCommands(std::vector<FXMenuCheck*>& commands) const {
    // save visible menu commands in commands vector
    if (menuCheckShowDemandElements->shown()) {
        commands.push_back(menuCheckShowDemandElements);
    }
    if (menuCheckSelectEdges->shown()) {
        commands.push_back(menuCheckSelectEdges);
    }
    if (menuCheckShowConnections->shown()) {
        commands.push_back(menuCheckShowConnections);
    }
    if (menuCheckHideConnections->shown()) {
        commands.push_back(menuCheckHideConnections);
    }
    if (menuCheckExtendSelection->shown()) {
        commands.push_back(menuCheckExtendSelection);
    }
    if (menuCheckChangeAllPhases->shown()) {
        commands.push_back(menuCheckChangeAllPhases);
    }
    if (menuCheckWarnAboutMerge->shown()) {
        commands.push_back(menuCheckWarnAboutMerge);
    }
    if (menuCheckShowJunctionBubble->shown()) {
        commands.push_back(menuCheckShowJunctionBubble);
    }
    if (menuCheckMoveElevation->shown()) {
        commands.push_back(menuCheckMoveElevation);
    }
    if (menuCheckChainEdges->shown()) {
        commands.push_back(menuCheckChainEdges);
    }
    if (menuCheckAutoOppositeEdge->shown()) {
        commands.push_back(menuCheckAutoOppositeEdge);
    }
}


bool
GNEViewNetHelper::NetworkViewOptions::showDemandElements() const {
    if (menuCheckShowDemandElements->shown()) {
        return (menuCheckShowDemandElements->getCheck() == TRUE);
    } else {
        // by default, if menuCheckShowDemandElements isn't shown, always show demand elements
        return true;
    }
}


bool
GNEViewNetHelper::NetworkViewOptions::selectEdges() const {
    if (menuCheckSelectEdges->shown()) {
        return (menuCheckSelectEdges->getCheck() == TRUE);
    } else {
        // by default, if menuCheckSelectEdges isn't shown, always select edges
        return true;
    }
}


bool
GNEViewNetHelper::NetworkViewOptions::showConnections() const {
    if (myViewNet->myEditModes.networkEditMode == NetworkEditMode::NETWORK_CONNECT) {
        // check if menu check hide connections ins shown
        return (menuCheckHideConnections->getCheck() == FALSE);
    } else if (myViewNet->myEditModes.networkEditMode == NetworkEditMode::NETWORK_PROHIBITION) {
        return true;
    } else if (menuCheckShowConnections->shown() == false) {
        return false;
    } else {
        return (myViewNet->getVisualisationSettings().showLane2Lane);
    }
}


bool
GNEViewNetHelper::NetworkViewOptions::editingElevation() const {
    if (menuCheckMoveElevation->shown()) {
        return (menuCheckMoveElevation->getCheck() == TRUE);
    } else {
        return false;
    }
}

// ---------------------------------------------------------------------------
// GNEViewNetHelper::DemandViewOptions - methods
// ---------------------------------------------------------------------------

GNEViewNetHelper::DemandViewOptions::DemandViewOptions(GNEViewNet* viewNet) :
    menuCheckHideShapes(nullptr),
    menuCheckHideNonInspectedDemandElements(nullptr),
    menuCheckShowAllPersonPlans(nullptr),
    menuCheckLockPerson(nullptr),
    myViewNet(viewNet),
    myLockedPerson(nullptr) {
}


void
GNEViewNetHelper::DemandViewOptions::buildDemandViewOptionsMenuChecks() {
    // create menu checks
    menuCheckHideShapes = new FXMenuCheck(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions,
        ("Hide shapes\t\tToggle show shapes (Polygons and POIs)"),
        myViewNet, MID_GNE_DEMANDVIEWOPTIONS_HIDESHAPES, LAYOUT_FIX_HEIGHT);
    menuCheckHideShapes->setHeight(23);
    menuCheckHideShapes->setCheck(false);
    menuCheckHideShapes->create();

    menuCheckHideNonInspectedDemandElements = new FXMenuCheck(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions,
        ("Hide non-inspected\t\tToggle show non-inspected demand elements"),
        myViewNet, MID_GNE_DEMANDVIEWOPTIONS_HIDENONINSPECTED, LAYOUT_FIX_HEIGHT);
    menuCheckHideNonInspectedDemandElements->setHeight(23);
    menuCheckHideNonInspectedDemandElements->setCheck(false);
    menuCheckHideNonInspectedDemandElements->create();

    menuCheckShowAllPersonPlans = new FXMenuCheck(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions,
        ("Show person plans\t\tshow all person plans"),
        myViewNet, MID_GNE_DEMANDVIEWOPTIONS_SHOWALLPERSONPLANS, LAYOUT_FIX_HEIGHT);
    menuCheckShowAllPersonPlans->setHeight(23);
    menuCheckShowAllPersonPlans->setCheck(false);
    menuCheckShowAllPersonPlans->create();

    menuCheckLockPerson = new FXMenuCheck(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions,
        ("Lock person\t\tLock selected person"),
        myViewNet, MID_GNE_DEMANDVIEWOPTIONS_LOCKPERSON, LAYOUT_FIX_HEIGHT);
    menuCheckLockPerson->setHeight(23);
    menuCheckLockPerson->setCheck(false);
    menuCheckLockPerson->create();

    // always recalc after creating new elements
    myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions->recalc();
}


void
GNEViewNetHelper::DemandViewOptions::hideDemandViewOptionsMenuChecks() {
    menuCheckHideShapes->hide();
    menuCheckHideNonInspectedDemandElements->hide();
    menuCheckShowAllPersonPlans->hide();
    menuCheckLockPerson->hide();
    // Also hide toolbar grip
    myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions->show();
}


void
GNEViewNetHelper::DemandViewOptions::getVisibleDemandMenuCommands(std::vector<FXMenuCheck*>& commands) const {
    // save visible menu commands in commands vector
    if (menuCheckHideShapes->shown()) {
        commands.push_back(menuCheckHideShapes);
    }
    if (menuCheckHideNonInspectedDemandElements->shown()) {
        commands.push_back(menuCheckHideNonInspectedDemandElements);
    }
    if (menuCheckShowAllPersonPlans->shown() && menuCheckShowAllPersonPlans->isEnabled()) {
        commands.push_back(menuCheckShowAllPersonPlans);
    }
    if (menuCheckLockPerson->shown() && menuCheckLockPerson->isEnabled()) {
        commands.push_back(menuCheckLockPerson);
    }
}


bool
GNEViewNetHelper::DemandViewOptions::showNonInspectedDemandElements(const GNEDemandElement* demandElement) const {
    if (menuCheckHideNonInspectedDemandElements->shown()) {
        // check conditions
        if ((menuCheckHideNonInspectedDemandElements->getCheck() == FALSE) || (myViewNet->getDottedAC() == nullptr)) {
            // if checkbox is disabled or there isn't insepected element, then return true
            return true;
        } else if (myViewNet->getDottedAC()->getTagProperty().isDemandElement()) {
            if (myViewNet->getDottedAC() == demandElement) {
                // if inspected element correspond to demandElement, return true
                return true;
            } else {
                // if demandElement is a route, check if dottedAC is one of their children (Vehicle or Stop)
                for (const auto& i : demandElement->getChildDemandElements()) {
                    if (i == myViewNet->getDottedAC()) {
                        return true;
                    }
                }
                // if demandElement is a vehicle, check if dottedAC is one of his route Parent
                for (const auto& i : demandElement->getParentDemandElements()) {
                    if (i == myViewNet->getDottedAC()) {
                        return true;
                    }
                }
                // dottedAC isn't one of their parent, then return false
                return false;
            }
        } else {
            // we're inspecting a demand element, then return true
            return true;
        }
    } else {
        // we're inspecting a demand element, then return true
        return true;
    }
}


bool
GNEViewNetHelper::DemandViewOptions::showShapes() const {
    if (menuCheckHideShapes->shown()) {
        return (menuCheckHideShapes->getCheck() == FALSE);
    } else {
        return true;
    }
}


bool
GNEViewNetHelper::DemandViewOptions::showAllPersonPlans() const {
    if (menuCheckShowAllPersonPlans->shown() && menuCheckShowAllPersonPlans->isEnabled()) {
        return (menuCheckShowAllPersonPlans->getCheck() == TRUE);
    } else {
        return false;
    }
}


void
GNEViewNetHelper::DemandViewOptions::lockPerson(const GNEDemandElement* person) {
    myLockedPerson = person;
}


void
GNEViewNetHelper::DemandViewOptions::unlockPerson() {
    myLockedPerson = nullptr;
}


const GNEDemandElement*
GNEViewNetHelper::DemandViewOptions::getLockedPerson() const {
    return myLockedPerson;
}

// ---------------------------------------------------------------------------
// GNEViewNetHelper::DataViewOptions - methods
// ---------------------------------------------------------------------------

GNEViewNetHelper::DataViewOptions::DataViewOptions(GNEViewNet* viewNet) :
    menuCheckShowDemandElements(nullptr),
    menuCheckHideShapes(nullptr),
    myViewNet(viewNet) {
}


void
GNEViewNetHelper::DataViewOptions::buildDataViewOptionsMenuChecks() {
    // create menu checks
    menuCheckShowDemandElements = new FXMenuCheck(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions,
        ("Show demand elements\t\tToggle show demand elements"),
        myViewNet, MID_GNE_DATAVIEWOPTIONS_SHOWDEMANDELEMENTS, LAYOUT_FIX_HEIGHT);
    menuCheckShowDemandElements->setHeight(23);
    menuCheckShowDemandElements->setCheck(false);
    menuCheckShowDemandElements->create();

    menuCheckHideShapes = new FXMenuCheck(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions,
        ("Hide shapes\t\tToggle show shapes (Polygons and POIs)"),
        myViewNet, MID_GNE_DATAVIEWOPTIONS_HIDESHAPES, LAYOUT_FIX_HEIGHT);
    menuCheckHideShapes->setHeight(23);
    menuCheckHideShapes->setCheck(false);
    menuCheckHideShapes->create();

    // always recalc after creating new elements
    myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions->recalc();
}


void
GNEViewNetHelper::DataViewOptions::hideDataViewOptionsMenuChecks() {
    menuCheckShowDemandElements->hide();
    menuCheckHideShapes->hide();
    // Also hide toolbar grip
    myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modeOptions->show();
}


void
GNEViewNetHelper::DataViewOptions::getVisibleDataMenuCommands(std::vector<FXMenuCheck*>& commands) const {
    // save visible menu commands in commands vector
    if (menuCheckShowDemandElements->shown()) {
        commands.push_back(menuCheckShowDemandElements);
    }
    if (menuCheckHideShapes->shown()) {
        commands.push_back(menuCheckHideShapes);
    }
}


bool
GNEViewNetHelper::DataViewOptions::showDemandElements() const {
    if (menuCheckShowDemandElements->shown()) {
        return (menuCheckShowDemandElements->getCheck() == TRUE);
    } else {
        // by default, if menuCheckShowDemandElements isn't shown, always show demand elements
        return true;
    }
}


bool
GNEViewNetHelper::DataViewOptions::showShapes() const {
    if (menuCheckHideShapes->shown()) {
        return (menuCheckHideShapes->getCheck() == FALSE);
    } else {
        return true;
    }
}

// ---------------------------------------------------------------------------
// GNEViewNetHelper::IntervalBar - methods
// ---------------------------------------------------------------------------

GNEViewNetHelper::IntervalBar::IntervalBar(GNEViewNet* viewNet) :
    myViewNet(viewNet),
    myDataSet(nullptr),
    myLimitByInterval(nullptr),
    myBeginTextField(nullptr),
    myEndTextField(nullptr) {
}


void
GNEViewNetHelper::IntervalBar::buildIntervalBarElements() {
    // create interval label
    FXLabel* dataSetLabel = new FXLabel(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().intervalBar,
        "Data sets", 0, GUIDesignLabelAttribute);
    dataSetLabel->create();
    // create combo box for sets
    myDataSet = new FXComboBox(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().intervalBar,
        GUIDesignComboBoxNCol, myViewNet, MID_GNE_DATASET_SELECTED, GUIDesignComboBoxWidth180);
    myDataSet->create();
    // create checkbutton for myLimitByInterval
    myLimitByInterval = new FXCheckButton(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().intervalBar,
        "Limit by interval", myViewNet, MID_GNE_DATAINTERVAL_LIMITED, GUIDesignCheckButtonLimitInterval);
    myLimitByInterval->create();
    // create textfield for begin
    myBeginTextField = new FXTextField(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().intervalBar,
        GUIDesignTextFieldNCol, myViewNet, MID_GNE_DATAINTERVAL_SETBEGIN, GUIDesignTextFielWidth50Real);
    myBeginTextField->setText("0");
    myBeginTextField->create();
    // create text field for end
    myEndTextField = new FXTextField(myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().intervalBar,
        GUIDesignTextFieldNCol, myViewNet, MID_GNE_DATAINTERVAL_SETEND, GUIDesignTextFielWidth50Real);
    myEndTextField->setText("3600");
    myEndTextField->create();
    // always recalc after creating new elements
    myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().intervalBar->recalc();
}


void
GNEViewNetHelper::IntervalBar::showIntervalBar() {
    // first update interval bar
    updateIntervalBar();
    // show toolbar grip
    myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().intervalBar->show();
}


void
GNEViewNetHelper::IntervalBar::hideIntervalBar() {
    // hide toolbar grip
    myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().intervalBar->hide();
}


void
GNEViewNetHelper::IntervalBar::updateLimitByInterval() {
    if (myLimitByInterval->isEnabled() && (myLimitByInterval->getCheck() == TRUE)) {
        myBeginTextField->enable();
        myEndTextField->enable();
    } else {
        myBeginTextField->disable();
        myEndTextField->disable();
    }
}


void
GNEViewNetHelper::IntervalBar::updateIntervalBar() {
    // first save current data set
    const std::string previousDataSet = myDataSet->getNumItems() > 0 ? myDataSet->getItem(myDataSet->getCurrentItem()).text() : "";
    // first clear items
    myDataSet->clearItems();
    if (myViewNet->getNet()) {
        // retrieve data sets
        const auto dataSets = myViewNet->getNet()->retrieveDataSets();
        if (dataSets.empty()) {
            myDataSet->appendItem("no data sets");
            // disable elements
            myDataSet->disable();
            myLimitByInterval->disable();
        } else {
            // declare integer to save previous data set index
            int previousDataSetIndex = 0;
            // enable elements
            myDataSet->enable();
            myLimitByInterval->enable();
            // add "<all>" item
            myDataSet->appendItem("<all>");
            // add all into
            for (const auto& dataSet : dataSets) {
                // check if current data set is the previous data set
                if (dataSet->getID() == previousDataSet) {
                    previousDataSetIndex = myDataSet->getNumItems();
                }
                myDataSet->appendItem(dataSet->getID().c_str());
            }
            // set visible elements
            if (myDataSet->getNumItems() < 10) {
                myDataSet->setNumVisible(myDataSet->getNumItems());
            } else {
                myDataSet->setNumVisible(10);
            }
            // set current data set
            myDataSet->setCurrentItem(previousDataSetIndex);
        }
        // update limit by interval
        updateLimitByInterval();
    }
}

// ---------------------------------------------------------------------------
// GNEViewNetHelper::CommonCheckableButtons - methods
// ---------------------------------------------------------------------------

GNEViewNetHelper::CommonCheckableButtons::CommonCheckableButtons(GNEViewNet* viewNet) :
    inspectButton(nullptr),
    deleteButton(nullptr),
    selectButton(nullptr),
    myViewNet(viewNet) {
}


void
GNEViewNetHelper::CommonCheckableButtons::buildCommonCheckableButtons() {
    // inspect button
    inspectButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tset inspect mode\tMode for inspect elements and change their attributes.",
        GUIIconSubSys::getIcon(GUIIcon::MODEINSPECT), myViewNet, MID_HOTKEY_I_INSPECTMODE, GUIDesignButtonToolbarCheckable);
    inspectButton->create();
    // delete button
    deleteButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tset delete mode\tMode for delete elements.",
        GUIIconSubSys::getIcon(GUIIcon::MODEDELETE), myViewNet, MID_HOTKEY_D_DELETEMODE, GUIDesignButtonToolbarCheckable);
    deleteButton->create();
    // select button
    selectButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tset select mode\tMode for select elements.",
        GUIIconSubSys::getIcon(GUIIcon::MODESELECT), myViewNet, MID_HOTKEY_S_SELECTMODE, GUIDesignButtonToolbarCheckable);
    selectButton->create();
    // always recalc menu bar after creating new elements
    myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes->recalc();
}


void
GNEViewNetHelper::CommonCheckableButtons::showCommonCheckableButtons() {
    inspectButton->show();
    deleteButton->show();
    selectButton->show();
}


void
GNEViewNetHelper::CommonCheckableButtons::hideCommonCheckableButtons() {
    inspectButton->hide();
    deleteButton->hide();
    selectButton->hide();
}


void
GNEViewNetHelper::CommonCheckableButtons::disableCommonCheckableButtons() {
    inspectButton->setChecked(false);
    deleteButton->setChecked(false);
    selectButton->setChecked(false);
}


void
GNEViewNetHelper::CommonCheckableButtons::updateCommonCheckableButtons() {
    inspectButton->update();
    deleteButton->update();
    selectButton->update();
}

// ---------------------------------------------------------------------------
// GNEViewNetHelper::NetworkCheckableButtons - methods
// ---------------------------------------------------------------------------

GNEViewNetHelper::NetworkCheckableButtons::NetworkCheckableButtons(GNEViewNet* viewNet) :
    moveNetworkElementsButton(nullptr),
    createEdgeButton(nullptr),
    connectionButton(nullptr),
    trafficLightButton(nullptr),
    additionalButton(nullptr),
    crossingButton(nullptr),
    TAZButton(nullptr),
    shapeButton(nullptr),
    prohibitionButton(nullptr),
    myViewNet(viewNet) {
}


void
GNEViewNetHelper::NetworkCheckableButtons::buildNetworkCheckableButtons() {
    // move button
    moveNetworkElementsButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tset move mode\tMode for move elements.",
        GUIIconSubSys::getIcon(GUIIcon::MODEMOVE), myViewNet, MID_HOTKEY_M_MOVEMODE, GUIDesignButtonToolbarCheckable);
    moveNetworkElementsButton->create();
    // create edge
    createEdgeButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tset create edge mode\tMode for creating junction and edges.",
        GUIIconSubSys::getIcon(GUIIcon::MODECREATEEDGE), myViewNet, MID_HOTKEY_E_EDGEMODE_EDGEDATAMODE, GUIDesignButtonToolbarCheckable);
    createEdgeButton->create();
    // connection mode
    connectionButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tset connection mode\tMode for edit connections between lanes.",
        GUIIconSubSys::getIcon(GUIIcon::MODECONNECTION), myViewNet, MID_HOTKEY_C_CONNECTMODE_PERSONPLANMODE, GUIDesignButtonToolbarCheckable);
    connectionButton->create();
    // prohibition mode
    prohibitionButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tset prohibition mode\tMode for editing connection prohibitions.",
        GUIIconSubSys::getIcon(GUIIcon::MODEPROHIBITION), myViewNet, MID_HOTKEY_W_PROHIBITIONMODE_PERSONTYPEMODE, GUIDesignButtonToolbarCheckable);
    prohibitionButton->create();
    // traffic light mode
    trafficLightButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tset traffic light mode\tMode for edit traffic lights over junctions.",
        GUIIconSubSys::getIcon(GUIIcon::MODETLS), myViewNet, MID_HOTKEY_T_TLSMODE_VTYPEMODE, GUIDesignButtonToolbarCheckable);
    trafficLightButton->create();
    // additional mode
    additionalButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tset additional mode\tMode for adding additional elements.",
        GUIIconSubSys::getIcon(GUIIcon::MODEADDITIONAL), myViewNet, MID_HOTKEY_A_ADDITIONALMODE_STOPMODE, GUIDesignButtonToolbarCheckable);
    additionalButton->create();
    // crossing mode
    crossingButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tset crossing mode\tMode for creating crossings between edges.",
        GUIIconSubSys::getIcon(GUIIcon::MODECROSSING), myViewNet, MID_HOTKEY_R_CROSSINGMODE_ROUTEMODE, GUIDesignButtonToolbarCheckable);
    crossingButton->create();
    // TAZ Mode
    TAZButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tset TAZ mode\tMode for creating Traffic Assignment Zones.",
        GUIIconSubSys::getIcon(GUIIcon::MODETAZ), myViewNet, MID_HOTKEY_Z_TAZMODE, GUIDesignButtonToolbarCheckable);
    TAZButton->create();
    // shape mode
    shapeButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tset polygon mode\tMode for creating polygons and POIs.",
        GUIIconSubSys::getIcon(GUIIcon::MODEPOLYGON), myViewNet, MID_HOTKEY_P_POLYGONMODE_PERSONMODE, GUIDesignButtonToolbarCheckable);
    shapeButton->create();
    // always recalc after creating new elements
    myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes->recalc();
}


void
GNEViewNetHelper::NetworkCheckableButtons::showNetworkCheckableButtons() {
    moveNetworkElementsButton->show();
    createEdgeButton->show();
    connectionButton->show();
    trafficLightButton->show();
    additionalButton->show();
    crossingButton->show();
    TAZButton->show();
    shapeButton->show();
    prohibitionButton->show();
}


void
GNEViewNetHelper::NetworkCheckableButtons::hideNetworkCheckableButtons() {
    moveNetworkElementsButton->hide();
    createEdgeButton->hide();
    connectionButton->hide();
    trafficLightButton->hide();
    additionalButton->hide();
    crossingButton->hide();
    TAZButton->hide();
    shapeButton->hide();
    prohibitionButton->hide();
}


void
GNEViewNetHelper::NetworkCheckableButtons::disableNetworkCheckableButtons() {
    moveNetworkElementsButton->setChecked(false);
    createEdgeButton->setChecked(false);
    connectionButton->setChecked(false);
    trafficLightButton->setChecked(false);
    additionalButton->setChecked(false);
    crossingButton->setChecked(false);
    TAZButton->setChecked(false);
    shapeButton->setChecked(false);
    prohibitionButton->setChecked(false);
}


void
GNEViewNetHelper::NetworkCheckableButtons::updateNetworkCheckableButtons() {
    moveNetworkElementsButton->update();
    createEdgeButton->update();
    connectionButton->update();
    trafficLightButton->update();
    additionalButton->update();
    crossingButton->update();
    TAZButton->update();
    shapeButton->update();
    prohibitionButton->update();
}

// ---------------------------------------------------------------------------
// GNEViewNetHelper::DemandCheckableButtons - methods
// ---------------------------------------------------------------------------

GNEViewNetHelper::DemandCheckableButtons::DemandCheckableButtons(GNEViewNet* viewNet) :
    moveDemandElementsButton(nullptr),
    routeButton(nullptr),
    vehicleButton(nullptr),
    vehicleTypeButton(nullptr),
    stopButton(nullptr),
    personTypeButton(nullptr),
    personButton(nullptr),
    personPlanButton(nullptr),
    myViewNet(viewNet) {
}


void
GNEViewNetHelper::DemandCheckableButtons::buildDemandCheckableButtons() {
    // move button
    moveDemandElementsButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tset move mode\tMode for move elements.",
        GUIIconSubSys::getIcon(GUIIcon::MODEMOVE), myViewNet, MID_HOTKEY_M_MOVEMODE, GUIDesignButtonToolbarCheckable);
    moveDemandElementsButton->create();
    // route mode
    routeButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tcreate route mode\tMode for creating routes.",
        GUIIconSubSys::getIcon(GUIIcon::MODEROUTE), myViewNet, MID_HOTKEY_R_CROSSINGMODE_ROUTEMODE, GUIDesignButtonToolbarCheckable);
    routeButton->create();
    // vehicle mode
    vehicleButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tcreate vehicle mode\tMode for creating vehicles.",
        GUIIconSubSys::getIcon(GUIIcon::MODEVEHICLE), myViewNet, MID_HOTKEY_V_VEHICLEMODE, GUIDesignButtonToolbarCheckable);
    vehicleButton->create();
    // vehicle type mode
    vehicleTypeButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tcreate vehicle type mode\tMode for creating vehicle types.",
        GUIIconSubSys::getIcon(GUIIcon::MODEVEHICLETYPE), myViewNet, MID_HOTKEY_T_TLSMODE_VTYPEMODE, GUIDesignButtonToolbarCheckable);
    vehicleTypeButton->create();
    // stop mode
    stopButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tcreate stop mode\tMode for creating stops.",
        GUIIconSubSys::getIcon(GUIIcon::MODESTOP), myViewNet, MID_HOTKEY_A_ADDITIONALMODE_STOPMODE, GUIDesignButtonToolbarCheckable);
    stopButton->create();
    // person type mode
    personTypeButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tcreate person type mode\tMode for creating person types.",
        GUIIconSubSys::getIcon(GUIIcon::MODEPERSONTYPE), myViewNet, MID_HOTKEY_W_PROHIBITIONMODE_PERSONTYPEMODE, GUIDesignButtonToolbarCheckable);
    personTypeButton->create();
    // person mode
    personButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tcreate person mode\tMode for creating persons.",
        GUIIconSubSys::getIcon(GUIIcon::MODEPERSON), myViewNet, MID_HOTKEY_P_POLYGONMODE_PERSONMODE, GUIDesignButtonToolbarCheckable);
    personButton->create();
    // person plan mode
    personPlanButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tcreate person plan mode\tMode for creating person plans.",
        GUIIconSubSys::getIcon(GUIIcon::MODEPERSONPLAN), myViewNet, MID_HOTKEY_C_CONNECTMODE_PERSONPLANMODE, GUIDesignButtonToolbarCheckable);
    personPlanButton->create();
    // always recalc after creating new elements
    myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes->recalc();
}


void
GNEViewNetHelper::DemandCheckableButtons::showDemandCheckableButtons() {
    moveDemandElementsButton->show();
    routeButton->show();
    vehicleButton->show();
    vehicleTypeButton->show();
    stopButton->show();
    personTypeButton->show();
    personButton->show();
    personPlanButton->show();
}


void
GNEViewNetHelper::DemandCheckableButtons::hideDemandCheckableButtons() {
    moveDemandElementsButton->hide();
    routeButton->hide();
    vehicleButton->hide();
    vehicleTypeButton->hide();
    stopButton->hide();
    personTypeButton->hide();
    personButton->hide();
    personPlanButton->hide();
}


void
GNEViewNetHelper::DemandCheckableButtons::disableDemandCheckableButtons() {
    moveDemandElementsButton->setChecked(false);
    routeButton->setChecked(false);
    vehicleButton->setChecked(false);
    vehicleTypeButton->setChecked(false);
    stopButton->setChecked(false);
    personTypeButton->setChecked(false);
    personButton->setChecked(false);
    personPlanButton->setChecked(false);
}


void
GNEViewNetHelper::DemandCheckableButtons::updateDemandCheckableButtons() {
    moveDemandElementsButton->update();
    routeButton->update();
    vehicleButton->update();
    vehicleTypeButton->update();
    stopButton->update();
    personTypeButton->update();
    personButton->update();
    personPlanButton->update();
}

// ---------------------------------------------------------------------------
// GNEViewNetHelper::DataCheckableButtons - methods
// ---------------------------------------------------------------------------

GNEViewNetHelper::DataCheckableButtons::DataCheckableButtons(GNEViewNet* viewNet) :
    edgeDataButton(nullptr),
    myViewNet(viewNet) {
}


void
GNEViewNetHelper::DataCheckableButtons::buildDataCheckableButtons() {
    // edgeData mode
    edgeDataButton = new MFXCheckableButton(false, myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes,
        "\tcreate edge data mode\tMode for creating edge datas.",
        GUIIconSubSys::getIcon(GUIIcon::MODEEDGEDATA), myViewNet, MID_HOTKEY_E_EDGEMODE_EDGEDATAMODE, GUIDesignButtonToolbarCheckable);
    edgeDataButton->create();
    // always recalc after creating new elements
    myViewNet->myViewParent->getGNEAppWindows()->getToolbarsGrip().modes->recalc();
}


void
GNEViewNetHelper::DataCheckableButtons::showDataCheckableButtons() {
    edgeDataButton->show();
}


void
GNEViewNetHelper::DataCheckableButtons::hideDataCheckableButtons() {
    edgeDataButton->hide();
}


void
GNEViewNetHelper::DataCheckableButtons::disableDataCheckableButtons() {
    edgeDataButton->setChecked(false);
}


void
GNEViewNetHelper::DataCheckableButtons::updateDataCheckableButtons() {
    edgeDataButton->update();
}

// ---------------------------------------------------------------------------
// GNEViewNetHelper::EditShapes - methods
// ---------------------------------------------------------------------------

GNEViewNetHelper::EditShapes::EditShapes(GNEViewNet* viewNet) :
    editedShapePoly(nullptr),
    editingNetworkElementShapes(false),
    myPreviousNetworkEditMode(NetworkEditMode::NETWORK_NONE),
    myViewNet(viewNet) {
}


void
GNEViewNetHelper::EditShapes::startEditCustomShape(GNENetworkElement* element, const PositionVector& shape, bool fill) {
    if ((editedShapePoly == nullptr) && (element != nullptr) && (shape.size() > 1)) {
        // save current edit mode before starting
        myPreviousNetworkEditMode = myViewNet->myEditModes.networkEditMode;
        if ((element->getTagProperty().getTag() == SUMO_TAG_CONNECTION) || (element->getTagProperty().getTag() == SUMO_TAG_CROSSING)) {
            editingNetworkElementShapes = true;
        } else {
            editingNetworkElementShapes = false;
        }
        // set move mode
        myViewNet->myEditModes.setNetworkEditMode(NetworkEditMode::NETWORK_MOVE);
        // add special GNEPoly fo edit shapes (color is taken from junction color settings)
        RGBColor col = myViewNet->getVisualisationSettings().junctionColorer.getSchemes()[0].getColor(3);
        editedShapePoly = myViewNet->myNet->addPolygonForEditShapes(element, shape, fill, col);
        // update view net to show the new editedShapePoly
        myViewNet->update();
    }
}


void
GNEViewNetHelper::EditShapes::stopEditCustomShape() {
    // stop edit shape junction deleting editedShapePoly
    if (editedShapePoly != nullptr) {
        myViewNet->myNet->removePolygonForEditShapes(editedShapePoly);
        editedShapePoly = nullptr;
        // restore previous edit mode
        if (myViewNet->myEditModes.networkEditMode != myPreviousNetworkEditMode) {
            myViewNet->myEditModes.setNetworkEditMode(myPreviousNetworkEditMode);
        }
    }
}


void
GNEViewNetHelper::EditShapes::saveEditedShape() {
    // save edited junction's shape
    if (editedShapePoly != nullptr) {
        myViewNet->myUndoList->p_begin("custom " + editedShapePoly->getShapeEditedElement()->getTagStr() + " shape");
        SumoXMLAttr attr = SUMO_ATTR_SHAPE;
        if (editedShapePoly->getShapeEditedElement()->getTagProperty().hasAttribute(SUMO_ATTR_CUSTOMSHAPE)) {
            attr = SUMO_ATTR_CUSTOMSHAPE;
        }
        editedShapePoly->getShapeEditedElement()->setAttribute(attr, toString(editedShapePoly->getShape()), myViewNet->myUndoList);
        myViewNet->myUndoList->p_end();
        stopEditCustomShape();
        myViewNet->update();
    }
}


/****************************************************************************/
