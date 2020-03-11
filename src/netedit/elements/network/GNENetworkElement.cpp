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
/// @file    GNENetworkElement.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jun 2016
///
// A abstract class for networkElements
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEViewParent.h>
#include <netedit/frames/common/GNESelectorFrame.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/geom/GeomHelper.h>

#include "GNENetworkElement.h"


// ===========================================================================
// method definitions
// ===========================================================================

GNENetworkElement::GNENetworkElement(GNENet* net, const std::string& id, GUIGlObjectType type, SumoXMLTag tag,
        const std::vector<GNEEdge*>& edgeParents,
        const std::vector<GNELane*>& laneParents,
        const std::vector<GNEShape*>& shapeParents,
        const std::vector<GNEAdditional*>& additionalParents,
        const std::vector<GNEDemandElement*>& demandElementParents,
        const std::vector<GNEGenericData*>& genericDataParents,
        const std::vector<GNEEdge*>& edgeChildren,
        const std::vector<GNELane*>& laneChildren,
        const std::vector<GNEShape*>& shapeChildren,
        const std::vector<GNEAdditional*>& additionalChildren,
        const std::vector<GNEDemandElement*>& demandElementChildren,
        const std::vector<GNEGenericData*>& genericDataChildren) :
    GUIGlObject(type, id),
    GNEAttributeCarrier(tag),
    GNEHierarchicalParentElements(this, edgeParents, laneParents, shapeParents, additionalParents, demandElementParents, genericDataParents),
    GNEHierarchicalChildElements(this, edgeChildren, laneChildren, shapeChildren, additionalChildren, demandElementChildren, genericDataChildren),
    myNet(net),
    myMovingGeometryBoundary() {
}


GNENetworkElement::~GNENetworkElement() {}


const std::string& 
GNENetworkElement::getID() const {
    return getMicrosimID();
}


void
GNENetworkElement::updateDottedGeometry(const PositionVector& shape) {
    myDottedGeometry.updateDottedGeometry(myNet->getViewNet()->getVisualisationSettings(), shape);
}


std::string
GNENetworkElement::generateChildID(SumoXMLTag /*childTag*/) {
    return "";
}


GNENet*
GNENetworkElement::getNet() const {
    return myNet;
}


GUIParameterTableWindow*
GNENetworkElement::getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView&) {
    // Create table
    GUIParameterTableWindow* ret = new GUIParameterTableWindow(app, *this);
    // Iterate over attributes
    for (const auto& i : myTagProperty) {
        // Add attribute and set it dynamic if aren't unique
        if (i.isUnique()) {
            ret->mkItem(i.getAttrStr().c_str(), false, getAttribute(i.getAttr()));
        } else {
            ret->mkItem(i.getAttrStr().c_str(), true, getAttribute(i.getAttr()));
        }
    }
    // close building
    ret->closeBuilding();
    return ret;
}


void
GNENetworkElement::selectAttributeCarrier(bool changeFlag) {
    if (!myNet) {
        throw ProcessError("Net cannot be nullptr");
    } else {
        gSelected.select(getGlID());
        // add object into list of selected objects
        myNet->getViewNet()->getViewParent()->getSelectorFrame()->getLockGLObjectTypes()->addedLockedObject(getType());
        if (changeFlag) {
            mySelected = true;

        }
    }
}


void
GNENetworkElement::unselectAttributeCarrier(bool changeFlag) {
    if (!myNet) {
        throw ProcessError("Net cannot be nullptr");
    } else {
        gSelected.deselect(getGlID());
        // remove object of list of selected objects
        myNet->getViewNet()->getViewParent()->getSelectorFrame()->getLockGLObjectTypes()->removeLockedObject(getType());
        if (changeFlag) {
            mySelected = false;
        }
    }
}


bool
GNENetworkElement::isAttributeCarrierSelected() const {
    return mySelected;
}


bool
GNENetworkElement::drawUsingSelectColor() const {
    if (mySelected && (myNet->getViewNet()->getEditModes().currentSupermode == Supermode::NETWORK)) {
        return true;
    } else {
        return false;
    }
}


void
GNENetworkElement::enableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    //
}


void
GNENetworkElement::disableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    //
}


std::string
GNENetworkElement::getPopUpID() const {
    if (myTagProperty.getTag() == SUMO_TAG_CONNECTION) {
        return getAttribute(SUMO_ATTR_FROM) + "_" + getAttribute(SUMO_ATTR_FROM_LANE) + " -> " + getAttribute(SUMO_ATTR_TO) + "_" + getAttribute(SUMO_ATTR_TO_LANE);
    } else {
        return getTagStr() + ": " + getID();
    }
}


std::string
GNENetworkElement::getHierarchyName() const {
    if (myTagProperty.getTag() == SUMO_TAG_LANE) {
        return toString(SUMO_TAG_LANE) + " " + getAttribute(SUMO_ATTR_INDEX);
    } else if (myTagProperty.getTag() == SUMO_TAG_CONNECTION) {
        return getAttribute(SUMO_ATTR_FROM_LANE) + " -> " + getAttribute(SUMO_ATTR_TO_LANE);
    } else if ((myTagProperty.getTag() == SUMO_TAG_EDGE) || (myTagProperty.getTag() == SUMO_TAG_CROSSING)) {
        return getPopUpID();
    } else {
        return getTagStr();
    }
}


void
GNENetworkElement::setEnabledAttribute(const int /*enabledAttributes*/) {
    //
}


/****************************************************************************/
