/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "dspInventoryAvailabilityByCustomerType.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "createCountTagsByItem.h"
#include "dspAllocations.h"
#include "dspOrders.h"
#include "dspReservations.h"
#include "dspRunningAvailability.h"
#include "dspSubstituteAvailabilityByItem.h"
#include "inputManager.h"
#include "mqlutil.h"
#include "purchaseOrder.h"
#include "reserveSalesOrderItem.h"
#include "storedProcErrorLookup.h"
#include "workOrder.h"

dspInventoryAvailabilityByCustomerType::dspInventoryAvailabilityByCustomerType(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_onlyShowShortages, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_showWoSupply, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_custtype, SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_avail, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));
  connect(_autoupdate, SIGNAL(toggled(bool)), this, SLOT(sAutoUpdateToggled(bool)));

  _custtype->setType(ParameterGroup::CustomerType);

  _avail->addColumn(tr("Item Number"),     _itemColumn, Qt::AlignLeft,  true, "item_number");
  _avail->addColumn(tr("Description"),              -1, Qt::AlignLeft,  true, "descrip");
  _avail->addColumn(tr("UOM"),              _uomColumn, Qt::AlignCenter,true, "uom_name");
  _avail->addColumn(tr("QOH"),              _qtyColumn, Qt::AlignRight, true, "qoh");
  _avail->addColumn(tr("This Alloc."),      _qtyColumn, Qt::AlignRight, true, "sobalance");
  _avail->addColumn(tr("Total Alloc."),     _qtyColumn, Qt::AlignRight, true, "allocated");
  _avail->addColumn(tr("Orders"),           _qtyColumn, Qt::AlignRight, true, "ordered");
  _avail->addColumn(tr("This Avail."),      _qtyColumn, Qt::AlignRight, true, "orderavail");
  _avail->addColumn(tr("Total Avail."),     _qtyColumn, Qt::AlignRight, true, "totalavail");
  _avail->addColumn(tr("At Shipping"),      _qtyColumn, Qt::AlignRight, true, "atshipping");
  _avail->addColumn(tr("Order/Start Date"),_dateColumn, Qt::AlignCenter,true, "orderdate");
  _avail->addColumn(tr("Sched./Due Date"), _dateColumn, Qt::AlignCenter,true, "duedate");
  _avail->setIndentation(10);

  if(!_metrics->boolean("EnableSOReservations"))
  {
    _useReservationNetting->hide();
    _useReservationNetting->setEnabled(false);
  }
  else
  {
    connect(_useReservationNetting, SIGNAL(toggled(bool)), this, SLOT(sHandleReservationNetting(bool)));
    if(_useReservationNetting->isChecked())
      sHandleReservationNetting(true);
  }
  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));
  if(_autoupdate->isChecked())
    sAutoUpdateToggled(true);

  sFillList();
}

dspInventoryAvailabilityByCustomerType::~dspInventoryAvailabilityByCustomerType()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspInventoryAvailabilityByCustomerType::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspInventoryAvailabilityByCustomerType::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("onlyShowShortages", &valid);
  if (valid)
    _onlyShowShortages->setChecked(TRUE);

  return NoError;
}

bool dspInventoryAvailabilityByCustomerType::setParams(ParameterList &params)
{
  _custtype->appendValue(params);

  if(_onlyShowShortages->isChecked())
    params.append("onlyShowShortages");
  if (_showWoSupply->isChecked())
    params.append("showWoSupply");
  if (_useReservationNetting->isChecked())
    params.append("useReservationNetting");

  return true;
}

void dspInventoryAvailabilityByCustomerType::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("InventoryAvailabilityByCustomerType", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspInventoryAvailabilityByCustomerType::sPopulateMenu(QMenu *pMenu,  QTreeWidgetItem *selected)
{
  int menuItem;
  
  if (_avail->altId() == -2)
  {
    menuItem = pMenu->insertItem(tr("Add to Packing List Batch"), this, SLOT(sAddToPackingListBatch()));
  }
  else if (_avail->altId() != -1)
  {
    menuItem = pMenu->insertItem("View Allocations...", this, SLOT(sViewAllocations()), 0);
    if (selected->text(5).toDouble() == 0.0)
      pMenu->setItemEnabled(menuItem, FALSE);
    
    menuItem = pMenu->insertItem("View Orders...", this, SLOT(sViewOrders()), 0);
    if (selected->text(6).toDouble() == 0.0)
     pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem("Running Availability...", this, SLOT(sRunningAvailability()), 0);
    menuItem = pMenu->insertItem("Substitute Availability...", this, SLOT(sViewSubstituteAvailability()), 0);

    q.prepare ("SELECT item_type "
             "FROM itemsite,item "
             "WHERE ((itemsite_id=:itemsite_id)"
             "AND (itemsite_item_id=item_id)"
             "AND (itemsite_supply));");
    q.bindValue(":itemsite_id", _avail->id());
    q.exec();
    if (q.next())
    {
      if (q.value("item_type") == "P")
      {
        pMenu->insertSeparator();
        menuItem = pMenu->insertItem("Issue Purchase Order...", this, SLOT(sIssuePO()), 0);
        if (!_privileges->check("MaintainPurchaseOrders"))
          pMenu->setItemEnabled(menuItem, FALSE);
      }
      else if (q.value("item_type") == "M")
      {
        pMenu->insertSeparator();
        menuItem = pMenu->insertItem("Issue Work Order...", this, SLOT(sIssueWO()), 0);
        if (!_privileges->check("MaintainWorkOrders"))
          pMenu->setItemEnabled(menuItem, FALSE);
      }
    }

    if(_metrics->boolean("EnableSOReservations"))
    {
      pMenu->insertSeparator();

      pMenu->insertItem(tr("Show Reservations..."), this, SLOT(sShowReservations()));
      pMenu->insertSeparator();

      int menuid;
      menuid = pMenu->insertItem(tr("Unreserve Stock"), this, SLOT(sUnreserveStock()), 0);
      pMenu->setItemEnabled(menuid, _privileges->check("MaintainReservations"));
      menuid = pMenu->insertItem(tr("Reserve Stock..."), this, SLOT(sReserveStock()), 0);
      pMenu->setItemEnabled(menuid, _privileges->check("MaintainReservations"));
      menuid = pMenu->insertItem(tr("Reserve Line Balance"), this, SLOT(sReserveLineBalance()), 0);
      pMenu->setItemEnabled(menuid, _privileges->check("MaintainReservations"));
    }

    pMenu->insertSeparator();
    menuItem = pMenu->insertItem("Issue Count Tag...", this, SLOT(sIssueCountTag()), 0);
    if (!_privileges->check("IssueCountTags"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspInventoryAvailabilityByCustomerType::sViewAllocations()
{
  q.prepare( "SELECT coitem_scheddate "
             "FROM coitem "
             "WHERE (coitem_id=:soitem_id);" );
  q.bindValue(":soitem_id", _avail->altId());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", _avail->id());
    params.append("byDate", q.value("coitem_scheddate"));
    params.append("run");

    dspAllocations *newdlg = new dspAllocations();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspInventoryAvailabilityByCustomerType::sViewOrders()
{
  q.prepare( "SELECT coitem_scheddate "
             "FROM coitem "
             "WHERE (coitem_id=:soitem_id);" );
  q.bindValue(":soitem_id", _avail->altId());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", _avail->id());
    params.append("byDate", q.value("coitem_scheddate"));
    params.append("run");

    dspOrders *newdlg = new dspOrders();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspInventoryAvailabilityByCustomerType::sRunningAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _avail->id());
  params.append("run");

  dspRunningAvailability *newdlg = new dspRunningAvailability();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByCustomerType::sViewSubstituteAvailability()
{
  q.prepare( "SELECT coitem_scheddate "
             "FROM coitem "
             "WHERE (coitem_id=:soitem_id);" );
  q.bindValue(":soitem_id", _avail->altId());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", _avail->id());
    params.append("byDate", q.value("coitem_scheddate"));
    params.append("run");

    dspSubstituteAvailabilityByItem *newdlg = new dspSubstituteAvailabilityByItem();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
//  ToDo
}

void dspInventoryAvailabilityByCustomerType::sIssuePO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _avail->id());

  purchaseOrder *newdlg = new purchaseOrder();
  if(newdlg->set(params) == NoError)
    omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByCustomerType::sIssueWO()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", _avail->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByCustomerType::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _avail->id());

  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspInventoryAvailabilityByCustomerType::sFillList()
{
  ParameterList params;             
  if (!setParams(params))
    return;
  MetaSQLQuery mql = mqlLoad("inventoryAvailability", "byCustOrSO");
  q = mql.toQuery(params);
  _avail->populate(q, true);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _avail->expandAll();
}

void dspInventoryAvailabilityByCustomerType::sAutoUpdateToggled(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}

void dspInventoryAvailabilityByCustomerType::sHandleReservationNetting(bool yn)
{
  if(yn)
    _avail->headerItem()->setText(7, tr("This Reserve"));
  else
    _avail->headerItem()->setText(7, tr("This Avail."));
  sFillList();
}

void dspInventoryAvailabilityByCustomerType::sReserveStock()
{
  ParameterList params;
  params.append("soitem_id", _avail->altId());

  reserveSalesOrderItem newdlg(this, "", true);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void dspInventoryAvailabilityByCustomerType::sReserveLineBalance()
{
  q.prepare("SELECT reserveSoLineBalance(:soitem_id) AS result;");
  q.bindValue(":soitem_id", _avail->altId());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("reserveSoLineBalance", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, tr("Error\n") +
                      q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void dspInventoryAvailabilityByCustomerType::sUnreserveStock()
{
  q.prepare("UPDATE coitem SET coitem_qtyreserved=0 WHERE coitem_id=:soitem_id;");
  q.bindValue(":soitem_id", _avail->altId());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, tr("Error\n") +
                      q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void dspInventoryAvailabilityByCustomerType::sShowReservations()
{
  ParameterList params;
  params.append("soitem_id", _avail->altId());
  params.append("run");

  dspReservations * newdlg = new dspReservations();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspInventoryAvailabilityByCustomerType::sAddToPackingListBatch()
{
  q.prepare("SELECT addToPackingListBatch(:sohead_id) AS result;");
  q.bindValue(":sohead_id", _avail->id());
  q.exec();
  sFillList();
}
