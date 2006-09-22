/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
		 2006 Martin Pfeiffer <hubipete@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef FRACTIONELEMENT_H
#define FRACTIONELEMENT_H

#include "BasicElement.h"


#include <QList>

namespace KFormula {
	
/**
 * @short A fraction element in a formula
 *
 * The fraction consists of two @see SequenceElement, the denominator and the numerator.
 * The SequenceElements can be set but actually altered they are with the
 * insertElementInNumerator() and insertElementInDenominator() methods.
 */
class FractionElement : public BasicElement {
public:
    /// The standard constructor
    FractionElement( BasicElement* parent = 0 );
   
    /// The standard destructor 
    ~FractionElement();

    /**
     * Insert a @see BasicElement in the numerator of the FractionElement
     * @param index The index in the @see SequenceElement where to insert @p element
     * @param element The @see BasicElement to insert in the numerator
     */
    void insertInNumerator( int index, BasicElement* element );

    /**
     * Insert a @see BasicElement in the denominator of the FractionElement
     * @param index The index in the @see SequenceElement where to insert @p element
     * @param element The @see BasicElement to insert in the denominator
     */
    void insertInDenominator( int index, BasicElement* element );
    
    /**
     * Obtain a list of all child elements of this element,
     * reimplementated from @see BasicElement
     * @return a QList with pointers to all child elements
     */
    const QList<BasicElement*> childElements();

    void readMathML( const QDomElement& element );
    
    void writeMathML( KoXmlWriter* writer, bool oasisFormat = false );





    enum { numeratorPos, denominatorPos };
	
    /**
     * Calculates our width and height and
     * our children's parentPosition.
     */
    virtual void calcSizes(const ContextStyle& style, ContextStyle::TextStyle tstyle, ContextStyle::IndexStyle istyle);

    /**
     * Draws the whole element including its children.
     * The `parentOrigin' is the point this element's parent starts.
     * We can use our parentPosition to get our own origin then.
     */
    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& style,
                       ContextStyle::TextStyle tstyle,
                       ContextStyle::IndexStyle istyle,
                       const LuPixelPoint& parentOrigin );

    /**
     * Enters this element while moving to the left starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or to the left of it.
     */
    virtual void moveLeft(FormulaCursor* cursor, BasicElement* from);

    /**
     * Enters this element while moving to the right starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or to the right of it.
     */
    virtual void moveRight(FormulaCursor* cursor, BasicElement* from);

    /**
     * Enters this element while moving up starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or above it.
     */
    virtual void moveUp(FormulaCursor* cursor, BasicElement* from);

    /**
     * Enters this element while moving down starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or below it.
     */
    virtual void moveDown(FormulaCursor* cursor, BasicElement* from);

    /**
     * Removes all selected children and returns them. Places the
     * cursor to where the children have been.
     *
     * We remove ourselve if we are requested to remove our numerator.
     *
     * It is possible to remove the denominator. But after this we
     * are senseless and the caller is required to replace us.
     */
    virtual void remove(FormulaCursor*, QList<BasicElement*>&, Direction);

    /// Sets the cursor to select the child. The mark is placed before, the position behind it.
    virtual void selectChild(FormulaCursor* cursor, BasicElement* child);

protected:
    /// Returns the tag name of this element type.
    virtual QString getTagName() const { return "FRACTION"; }

    /// Appends our attributes to the dom element.
    virtual void writeDom(QDomElement element);

    /// Reads our attributes from the element. Returns false if it failed.
    virtual bool readAttributesFromDom(QDomElement element);

    /**
     * Reads our content from the node. Sets the node to the next node
     * that needs to be read. Returns false if it failed.
     */
    virtual bool readContentFromDom(QDomNode& node);

private:
    BasicElement* m_numerator;
    BasicElement* m_denominator;
};

} // namespace KFormula

#endif // FRACTIONELEMENT_H
