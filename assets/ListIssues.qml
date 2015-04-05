import bb.cascades 1.3
import Lib.BugReport 1.0


Page {
    
    property int typeIssue
    
    Container {
        ListView {
            id: currentIssuesList
            
            
        }
        
    }
    
    onCreationCompleted: {
        bugReportController.setListView(currentIssuesList);
    }
    
    onTypeIssueChanged: {
        bugReportController.loadIssues(typeIssue);
    }
    
    attachedObjects: [
        BugReport {
            id: bugReportController
            
        }
    ]
}
