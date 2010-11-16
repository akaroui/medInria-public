#include "medPacsMover.h"

#include "medAbstractPacsMoveScu.h"
#include "medAbstractPacsFactory.h"
#include "medAbstractPacsNode.h"
#include "medPacsNode.h"

class medPacsMoverPrivate
{
public:

    medAbstractPacsMoveScu* move;

    QVector<medMoveCommandItem> cmdList;
};

medPacsMover::medPacsMover(const QVector<medMoveCommandItem>& cmdList): QRunnable(), 
                           d(new medPacsMoverPrivate)
{
    d->cmdList = cmdList;

    d->move = NULL;
}

medPacsMover::~medPacsMover( void )
{
    if (d->move) delete d->move;
    delete d;

    d = NULL;
}

void medPacsMover::run( void )
{
    doQueuedMove();
}

void medPacsMover::doQueuedMove()
{
    if(!d->move)
    {
        d->move = medAbstractPacsFactory::instance()->createMoveScu("dcmtkMoveScu");
        connect(d->move,SIGNAL(progressed(int)),this, SIGNAL(progressed(int)));
    }

    if(d->move)
    {
        for(int i=0; i<d->cmdList.size(); i++)
        {
            medPacsNode source;
            source.setTitle(d->cmdList.at(i).sourceTitle);
            source.setIp(d->cmdList.at(i).sourceIp);
            source.setPort(d->cmdList.at(i).sourcePort);

            medPacsNode target;
            target.setTitle(d->cmdList.at(i).targetTitle);
            target.setIp(d->cmdList.at(i).targetIp);
            target.setPort(d->cmdList.at(i).targetPort);

            d->move->addRequestToQueue(d->cmdList.at(i).group, d->cmdList.at(i).elem, d->cmdList.at(i).query.toLatin1(), source, target);
        }
    
    if(d->move->performQueuedMoveRequests() == 0)
            emit success();
    else
            emit failure();

    }

}

void medPacsMover::onCancel()
{
    if(d->move)
    {
        d->move->sendCancelRequest();
    }
    emit cancelled();
}
