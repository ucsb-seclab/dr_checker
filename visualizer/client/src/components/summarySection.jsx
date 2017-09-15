
import React from 'react';
import PropTypes from 'prop-types';
import Table, { TableBody, TableCell, TableHead, TableRow } from 'material-ui/Table';
import Checkbox from 'material-ui/Checkbox';
import Icon from 'material-ui/Icon';
import Button from 'material-ui/Button';
import { withStyles } from 'material-ui/styles';
import SummaryInfo from './summaryInfo.jsx';

const styles = theme => ({
  gridTableSummary: {
    borderLeft: `1px solid ${theme.palette.primary[500]}`,
  },
});

/**
 * This class holds and manage the information regarding the summary 
 * of the selected analysis00
 */
class SummarySection extends React.PureComponent {
  constructor() {
    super();
    this.state = {
      infoDialogOpen: false,
      infoDialogTitle: '',
      infoDialogContent: '',
    };
  }
  /** 
   * Select / desect all the entry inside the table
   */
  handleSelectAllClick = (event, checked) => {
    let selected = [];
    if (checked) {
      selected = this.props.data.map(n => n.id);
    }
    this.props.handleSelection(selected);
  }

  /** 
   * Handle the click on the checkbox
   */
  handleSelectClick = (event, id) => {
    const { selected } = this.props;
    const selectedIndex = selected.indexOf(id);
    let newSelected = [];

    if (selectedIndex === -1) {
      newSelected = newSelected.concat(selected, id);
    } else if (selectedIndex === 0) {
      newSelected = newSelected.concat(selected.slice(1));
    } else if (selectedIndex === selected.length - 1) {
      newSelected = newSelected.concat(selected.slice(0, -1));
    } else if (selectedIndex > 0) {
      newSelected = newSelected.concat(
        selected.slice(0, selectedIndex),
        selected.slice(selectedIndex + 1),
      );
    }
    this.props.handleSelection(newSelected);
  };

  /**
   * Check if the id is selected or not
   */
  isSelected = id => this.props.selected.indexOf(id) !== -1;

  openInfoDialog = (title, content) => {
    this.setState({
      infoDialogContent: content,
      infoDialogOpen: true,
      infoDialogTitle: title,
    });
  }

  closeInfoDalog = () => {
    this.setState({
      infoDialogOpen: false,
    });
  }

  render() {
    return (
      <div>
        <Table>
          <TableHead>
            <TableRow>
              <TableCell checkbox>
                <Checkbox
                  indeterminate={this.props.selected.length > 0 &&
                               this.props.selected.length < this.props.data.length}
                  checked={this.props.selected.length === this.props.data.length}
                  onChange={this.handleSelectAllClick}
                />
              </TableCell>
              <TableCell disablePadding>Warning Type</TableCell>
              <TableCell numeric>Actions</TableCell>
              <TableCell numeric>Color</TableCell>
            </TableRow>
          </TableHead>
          <TableBody>
            {this.props.data.map((n) => {
              const isSelected = this.isSelected(n.id);
              return (
                <TableRow
                  role="checkbox"
                  aria-checked={isSelected}
                  tabIndex={-1}
                  key={n.id}
                  selected={isSelected}
                >
                  <TableCell checkbox>
                    <Checkbox
                      checked={isSelected} 
                      onClick={event => this.handleSelectClick(event, n.id)}
                    />
                  </TableCell>
                  <TableCell disablePadding>{n.name}</TableCell>
                  <TableCell numeric>
                    <Icon onClick={() => this.openInfoDialog(n.name, n.info)}>search</Icon>
                  </TableCell>
                  <TableCell numeric>
                    <Icon style={{ color: n.color }}>label</Icon>
                  </TableCell>
                </TableRow>
              );
            })}
          </TableBody>
        </Table>
        <SummaryInfo
          open={this.state.infoDialogOpen}
          title={this.state.infoDialogTitle}
          content={this.state.infoDialogContent}
          onRequestClose={this.closeInfoDalog}
        />
      </div>
    );
  }
}

SummarySection.propTypes = {
  selected: PropTypes.arrayOf(PropTypes.number).isRequired,
  handleSelection: PropTypes.func.isRequired,
  data: PropTypes.arrayOf(PropTypes.object).isRequired,
};

export default withStyles(styles)(SummarySection);
