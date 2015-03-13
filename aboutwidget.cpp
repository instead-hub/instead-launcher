#include "config.h"
#include "aboutwidget.h"

AboutWidget::AboutWidget( QWidget *parent )
    : QWidget( parent )
{
    QVBoxLayout *layout = new QVBoxLayout( this );
    QHBoxLayout *hlayout = new QHBoxLayout();

    QWidget *gwidget = new QWidget( this );
    QPalette pal = gwidget->palette();
    pal.setBrush( gwidget->backgroundRole(), QBrush( QColor( 255, 255, 255 ) ) );
    gwidget->setPalette( pal );
    gwidget->setAutoFillBackground( true );

    QVBoxLayout *glayout = new QVBoxLayout( gwidget );
//    glayout->setSizeConstraint( QLayout::SetFixedSize );

    QWidget *lwidget = new QWidget( this );
    pal = lwidget->palette();
    pal.setBrush( lwidget->backgroundRole(), QBrush( QColor( 150, 150, 150 ) ) );
    lwidget->setPalette( pal );
    lwidget->setAutoFillBackground( true );

    QWidget *iwidget = new QWidget( gwidget );
    iwidget->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );

    QVBoxLayout *ghlvlayout = new QVBoxLayout( lwidget );

    QHBoxLayout *ghlayout = new QHBoxLayout();
    QVBoxLayout *ghvlayout = new QVBoxLayout( iwidget );
    QHBoxLayout *ghblayout = new QHBoxLayout();
    layout->addLayout( hlayout );
//    hlayout->addStretch();
    hlayout->addWidget( gwidget );
//    hlayout->addStretch();
    glayout->addLayout( ghlayout );
    m_logoLabel = new QLabel( this );
    ghlvlayout->addWidget( m_logoLabel );

    m_logoLabel->setPixmap( QPixmap( ":/resources/logo.png" ) );
    ghlayout->addWidget( lwidget );
    ghlayout->addWidget( iwidget );
    QFrame *frame = new QFrame( iwidget );
    frame->setFrameShape( QFrame::HLine );
    ghvlayout->addWidget( frame );
    QLabel *title = new QLabel( "<b><h3>" + QString( "%1 %2" ).arg( tr( "INSTEAD launcher" ) ).arg( LAUNCHER_VERSION ) + "</h3></b>", this );
    title->setScaledContents( false );
    ghvlayout->addWidget( title );
    frame = new QFrame( iwidget );
    frame->setFrameShape( QFrame::HLine );
    ghvlayout->addWidget( frame );
    QLabel *desc = new QLabel( tr( "The program for loading and installing games from the official INSTEAD games repository. Also launcher allows you to be in course of all related software updates." ), this );
    desc->setWordWrap( true );
    desc->setScaledContents( false );
    ghvlayout->addSpacing( 10 );
    ghvlayout->addWidget( desc );
    QCommandLinkButton *developersForumButton = new QCommandLinkButton( tr( "Developer's forum" ), tr( "Join us! Let's discuss, make suggestions and enjoy!" ), lwidget );
//    developersForumButton->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding ) );
//    developersForumButton->setMinimumHeight( 150 );
    connect( developersForumButton, SIGNAL( clicked() ), this, SLOT( goToDevelopersForum() ) );
    ghvlayout->addWidget( developersForumButton );
    QCommandLinkButton *launcherProjectButton = new QCommandLinkButton( tr( "INSTEAD launcher project page" ), tr( "If you want to report about bug, you can do it here." ), lwidget );
//    launcherProjectButton->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed ) );
    connect( launcherProjectButton, SIGNAL( clicked() ), this, SLOT( goToINSTEADLauncherPage() ) );
    ghvlayout->addWidget( launcherProjectButton );
    QCommandLinkButton *insteadProjectButton = new QCommandLinkButton( tr( "INSTEAD project page" ), tr( "Welcome to the INSTEAD project page!" ), lwidget );
//    insteadProjectButton->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed ) );
    connect( insteadProjectButton, SIGNAL( clicked() ), this, SLOT( goToINSTEADPage() ) );
    ghvlayout->addWidget( insteadProjectButton );
    QLabel *devs = new QLabel( "<center><b>" + tr( "Developers team" ) + ":</b><br>morkow, rec, <a href=\"mailto:ilja.ryndin@gmail.com\">" + tr( "Ilja Ryndin" ) +"</a></center>", this );
    devs->setWordWrap( true );
    devs->setScaledContents( false );
    ghvlayout->addSpacing( 10 );
//    ghlvlayout->addWidget( devs );
    ghvlayout->addWidget( devs );
    connect( devs, SIGNAL( linkActivated( const QString & ) ), this, SLOT( linkActivated( const QString & ) ) );

    QLabel *thanks = new QLabel( "<center><b>" + tr( "Special thanks to" ) + ":</b><br>" +
    	tr("Peter Kosyh for the smart advices and everybody who was involed in developmnt process! :)") + "</center>", 
    	this );
    thanks->setWordWrap( true );
    ghvlayout->addSpacing( 10 );
//    ghlvlayout->addWidget( thanks );
    ghvlayout->addWidget( thanks );

    ghvlayout->addStretch();

    ghvlayout->addLayout( ghblayout );
    ghblayout->addStretch();
    QPushButton *checkUpdatesButton = new QPushButton( tr( "Check software updates" ), iwidget );
    ghblayout->addWidget( checkUpdatesButton );
    ghblayout->addStretch();

    ghlvlayout->addStretch();

    connect( checkUpdatesButton, SIGNAL( clicked() ), this, SIGNAL( checkUpdatesRequest() ) );


}

AboutWidget::~AboutWidget()
{
}

void AboutWidget::linkActivated( const QString &link ) {
    QDesktopServices::openUrl( link );
}

void AboutWidget::goToDevelopersForum() {
    QDesktopServices::openUrl( QString( "http://instead.syscall.ru/talk" ) );
}

void AboutWidget::goToINSTEADPage() {
    QDesktopServices::openUrl( QString( "http://instead.sourceforge.net" ) );
}

void AboutWidget::goToINSTEADLauncherPage() {
    QDesktopServices::openUrl( QString( "https://github.com/instead-hub/instead-launcher" ) );
}
