import bb.cascades 1.3

Page {
    property int number
    
    Container {
        layout: StackLayout {
            orientation: LayoutOrientation.TopToBottom
        }
        
        
        ScrollView {
            WebView {
                id: issueWebView
                
                settings.textAutosizingEnabled: false
                settings.zoomToFitEnabled: false
            }            
        }
        
    }
    
    onNumberChanged: {
        bugReportController.loadIssue(number);
    }
    
    onCreationCompleted: {
        bugReportController.setWebView(issueWebView);
    }
}
