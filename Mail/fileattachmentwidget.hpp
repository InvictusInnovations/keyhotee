#ifndef FILEATTACHMENTWIDGET_H
#define FILEATTACHMENTWIDGET_H

#include <QWidget>

namespace Ui {
class TFileAttachmentWidget;
}

/** Implements file attachment widget providing file attach GUI functionality to the email editor.
*/
class TFileAttachmentWidget : public QWidget
  {
  Q_OBJECT

  public:
    explicit TFileAttachmentWidget(QWidget* parent);
    virtual ~TFileAttachmentWidget();

  private:
    /// Allows to prebuild context menu specific to attachment table.
    void ConfigureContextMenu();
    /// Configures other properties of attachment table.
    void ConfigureAttachmentTable();
    void UpdateColumnHeaders(unsigned int count, unsigned long long totalSize);

  private slots:
    void onAddTriggered();
    void onDelTriggered();
    void onSaveTriggered();
    void onAttachementTableSelectionChanged();

  private:
    Ui::TFileAttachmentWidget* ui;
//    QMenu*                     ContextMenu;
    QStringList                SelectedFiles;

  };

#endif // FILEATTACHMENTWIDGET_H

