using System.ComponentModel;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace DataBinding
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class Scenario9 
    {
         // A pointer back to the main page.  This is needed if you want to call methods in MainPage such
        // as NotifyUser()

        Employee _employee;

        public Scenario9()
        {
            this.InitializeComponent();
            ScenarioReset(null, null);
        }
        
        private void ScenarioReset(object sender, RoutedEventArgs e)
        {
            _employee = new Employee();
            _employee.Name = "Jane Doe";
            _employee.Organization = "Contoso";
            _employee.PropertyChanged += employeeChanged;

            Output.DataContext = _employee;
            tbBoundDataModelStatus.Text = "";
        }
              
        private void employeeChanged(object sender, PropertyChangedEventArgs e)
        {
            tbBoundDataModelStatus.Text = "The property:'" + e.PropertyName + "' was changed";
        }

        private void UpdateDataBtnClick(object sender, RoutedEventArgs e)
        {
            var expression = NameTxtBox.GetBindingExpression(TextBox.TextProperty);
            expression.UpdateSource();
        }

    }
}
