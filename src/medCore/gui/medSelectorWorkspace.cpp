/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include <medSelectorToolBox.h>
#include <medSelectorWorkspace.h>

class medSelectorWorkspacePrivate
{
public:
    // Give values to items without a constructor.
    medSelectorWorkspacePrivate() :
       selectorToolBox(NULL)
    {}

    QPointer<medSelectorToolBox> selectorToolBox;
};

medSelectorWorkspace::medSelectorWorkspace(QWidget * parent, QString name, medSelectorToolBox* toolbox) : medAbstractWorkspace(parent), d(new medSelectorWorkspacePrivate)
{
    if(!toolbox)
    {
        d->selectorToolBox = new medSelectorToolBox(parent, name);
    }
    else
    {
        d->selectorToolBox = toolbox;
    }

    connect(d->selectorToolBox,SIGNAL(success()),this,SLOT(onProcessSuccess()));

    this->addToolBox(d->selectorToolBox);
    d->selectorToolBox->setTitle(name); // get workspace name
}

medSelectorWorkspace::~medSelectorWorkspace(void)
{
    delete d;
    d = NULL;
}

medSelectorToolBox* medSelectorWorkspace::selectorToolBox()
{
    return d->selectorToolBox;
}
