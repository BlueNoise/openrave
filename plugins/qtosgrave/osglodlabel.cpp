#include "osglodlabel.h"
#include "qtosg.h"
#include <iostream>
#include <osg/Depth>

namespace qtosgrave {

// OSG text label that scales by camera distance and also disappears if far away enough
OSGLODLabel::OSGLODLabel(const std::string& label) : osg::LOD() {
    /* Transform structure of an OSGLODLabel: 
    *
    * [Target Transform (usually the global transform)]
    *           |
    *           |
    * [Label Offset Transform (optional)]
    *           |
    *           |
    * [Global LOD (this + controls label visibility)]
    *           |
    *           |
    *     [Label Geode]
    *           |
    *           |
    *     [Label Text]
    */

    // Set up text element and its properties
    osg::ref_ptr<osgText::Text> text = new osgText::Text();
    text->setText(label);
    text->setCharacterSize(0.05);
    text->setAutoRotateToScreen(true);
    text->setFont( "/usr/share/fonts/truetype/msttcorefonts/Arial.ttf" );
    text->setPosition( osg::Vec3( 0.0, 0.0, 0.0 ) );
    text->getOrCreateStateSet()->removeAttribute((new osg::Program)->getType(), 1);
    text->setDrawMode( osgText::Text::TEXT );
    text->setColor(osg::Vec4(0,0,0,1));
    text->setBackdropColor( osg::Vec4( 1.0, 1.0f, 1.0f, 1.0f ) );
    text->setBackdropType( osgText::Text::OUTLINE );
    text->setAlignment( osgText::Text::CENTER_CENTER );
    text->setAxisAlignment( osgText::Text::SCREEN );
    text->setCharacterSizeMode( osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT );
    text->setMaximumHeight(0.5);
    text->setFontResolution(128,128);

    // Override the cartoon shader with the default shader for text
    osg::ref_ptr<osg::Program> program = new osg::Program;
    text->getOrCreateStateSet()->setAttributeAndModes(program.get(), osg::StateAttribute::PROTECTED | osg::StateAttribute::ON);

    // Ensure that the text is drawn over any shape in the scene
    text->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::PROTECTED | osg::StateAttribute::OFF );
    text->getOrCreateStateSet()->setAttributeAndModes( new osg::Depth(osg::Depth::ALWAYS), osg::StateAttribute::PROTECTED | osg::StateAttribute::ON);
    text->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::PROTECTED | osg::StateAttribute::OFF );

    // Place text element into geode node
    osg::ref_ptr<osg::Geode> textGeode = new osg::Geode();
    textGeode->addDrawable(text);

    // Place geode node into LOD node for distance-based culling
    this->addChild(textGeode, 0, 20);
}

// Debug print during label destruction for testing
OSGLODLabel::~OSGLODLabel() {
    // RAVELOG_DEBUG("Destroying OSGLODLabel!");
}

// Overriding of default LOD node traverse
void OSGLODLabel::traverse(osg::NodeVisitor& nv)
{
    // Use global distance instead of local distance from camera if node visitor settings are applicable
    // Otherwise, use the default LOD node implementation of this function
    if (nv.getTraversalMode() == osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN && _rangeMode==DISTANCE_FROM_EYE_POINT) {
        float requiredRange = nv.getDistanceToViewPoint(getCenter(),true);
        // The input node visitor, represented as a CullStack
        osg::CullStack *cullStack = dynamic_cast<osg::CullStack *>(&nv);
        if (cullStack != NULL) {
            // Use cull stack's model view matrix to obtain world distance to camera
            osg::RefMatrix* modelView = cullStack->getModelViewMatrix();
            // We use the last column of the modelview matrix to acquire object space camera translation

            // Note that OSG matrix accesses are transposed, i.e. we access the matrix
            // with (col, row) rather than (row, col)
            requiredRange = osg::Vec3d((*modelView)(3,0), (*modelView)(3,1), (*modelView)(3,2)).length();
        }

        unsigned int numChildren = _children.size();
        if (_rangeList.size()<numChildren) numChildren=_rangeList.size();
        for(unsigned int i=0;i<numChildren;++i)
        {
            if (_rangeList[i].first<=requiredRange && requiredRange<_rangeList[i].second)
            {
                _children[i]->accept(nv);
            }
        }
    } else {
        osg::LOD::traverse(nv);
    }
}

}