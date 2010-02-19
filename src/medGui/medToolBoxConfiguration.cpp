/* medToolBoxConfiguration.cpp --- 
 * 
 * Author: Julien Wintz
 * Copyright (C) 2008 - Julien Wintz, Inria.
 * Created: Fri Feb 19 09:06:02 2010 (+0100)
 * Version: $Id$
 * Last-Updated: Fri Feb 19 10:54:25 2010 (+0100)
 *           By: Julien Wintz
 *     Update #: 10
 */

/* Commentary: 
 * 
 */

/* Change log:
 * 
 */

#include "medToolBoxConfiguration.h"

class medToolBoxConfigurationPrivate
{
public:
    QComboBox *combo;
};

medToolBoxConfiguration::medToolBoxConfiguration(QWidget *parent) : medToolBox(parent), d(new medToolBoxConfigurationPrivate)
{
    QWidget *central = new QWidget(this);

    d->combo = new QComboBox(this);
    d->combo->addItem("Default");
    d->combo->addItem("Diffusion");
    d->combo->addItem("Registration");

    QHBoxLayout *layout = new QHBoxLayout(central);
    layout->addWidget(d->combo);

    this->setTitle("Configuration");
    this->setWidget(central);

    connect(d->combo, SIGNAL(currentIndexChanged(QString)), this, SIGNAL(configurationChanged(QString)));
}

medToolBoxConfiguration::~medToolBoxConfiguration(void)
{
    delete d;

    d = NULL;
}