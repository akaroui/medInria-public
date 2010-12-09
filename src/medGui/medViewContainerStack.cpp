/* medViewContainerStack.cpp --- 
 * 
 * Author: Julien Wintz
 * Copyright (C) 2008 - Julien Wintz, Inria.
 * Created: Thu May 13 12:39:09 2010 (+0200)
 * Version: $Id$
 * Last-Updated: Tue Jun 15 22:12:57 2010 (+0200)
 *           By: Julien Wintz
 *     Update #: 8
 */

/* Commentary: 
 * 
 */

/* Change log:
 * 
 */

#include "medViewContainerStack.h"

#include <medGui/medViewContainer.h>
#include <medGui/medViewContainerCustom.h>
#include <medGui/medViewContainerMulti.h>
#include <medGui/medViewContainerSingle.h>

class medViewContainerStackPrivate
{
public:
    medViewContainerSingle *container_single;
    medViewContainerMulti *container_multi;
    medViewContainerCustom *container_custom;
    medViewContainer *container_registration_compare;
    medViewContainer *container_registration_fuse;
};

medViewContainerStack::medViewContainerStack(QWidget *parent) : QStackedWidget(parent), d(new medViewContainerStackPrivate)
{
    d->container_single = new medViewContainerSingle(this);
    d->container_multi = new medViewContainerMulti(this);
    d->container_custom = new medViewContainerCustom(this);
    d->container_registration_compare = new medViewContainerCustom(this);
    d->container_registration_compare->split(1, 2);
    d->container_registration_fuse = new medViewContainerCustom(this);

    this->addWidget(d->container_single);
    this->addWidget(d->container_multi);
    this->addWidget(d->container_custom);
    this->addWidget(d->container_registration_compare);
    this->addWidget(d->container_registration_fuse);

    this->setCurrentIndex(0);

    connect(d->container_single, SIGNAL(focused(dtkAbstractView*)), this, SIGNAL(focused(dtkAbstractView*)));
    connect(d->container_multi,  SIGNAL(focused(dtkAbstractView*)), this, SIGNAL(focused(dtkAbstractView*)));
    connect(d->container_custom, SIGNAL(focused(dtkAbstractView*)), this, SIGNAL(focused(dtkAbstractView*)));
    connect(d->container_registration_compare, SIGNAL(focused(dtkAbstractView*)), this, SIGNAL(focused(dtkAbstractView*)));
    connect(d->container_registration_fuse, SIGNAL(focused(dtkAbstractView*)), this, SIGNAL(focused(dtkAbstractView*)));

    connect(d->container_single, SIGNAL(dropped(const medDataIndex&)), this, SIGNAL(dropped(const medDataIndex&)));
    connect(d->container_multi,  SIGNAL(dropped(const medDataIndex&)), this, SIGNAL(dropped(const medDataIndex&)));
    connect(d->container_custom, SIGNAL(dropped(const medDataIndex&)), this, SIGNAL(dropped(const medDataIndex&)));
    connect(d->container_registration_compare, SIGNAL(dropped(const medDataIndex&)), this, SIGNAL(dropped(const medDataIndex&)));
    connect(d->container_registration_fuse, SIGNAL(dropped(const medDataIndex&)), this, SIGNAL(dropped(const medDataIndex&)));
}

medViewContainerStack::~medViewContainerStack(void)
{
    delete d;

    d = NULL;
}

medViewContainer *medViewContainerStack::current(void)
{
    if(this->currentIndex() == 0)
        return d->container_single;

    if(this->currentIndex() == 1)
        return d->container_multi;

    if(this->currentIndex() == 2)
        return d->container_custom;
	
    if(this->currentIndex() == 3)
        return d->container_registration_compare;
    
    if(this->currentIndex() == 4)
        return d->container_registration_fuse;

    return NULL;
}

medViewContainer *medViewContainerStack::single(void)
{
    return d->container_single;
}

medViewContainer *medViewContainerStack::multi(void)
{
    return d->container_multi;
}

medViewContainer *medViewContainerStack::custom(void)
{
    return d->container_custom;
}

medViewContainer *medViewContainerStack::compare(void)
{
    return d->container_registration_compare;
}

medViewContainer *medViewContainerStack::fuse(void)
{
    return d->container_registration_fuse;
}