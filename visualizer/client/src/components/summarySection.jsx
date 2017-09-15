
import React from 'react';
import PropTypes from 'prop-types';
import Table, { TableBody, TableCell, TableHead, TableRow } from 'material-ui/Table';
import Checkbox from 'material-ui/Checkbox';
import Icon from 'material-ui/Icon';
import { withStyles } from 'material-ui/styles';

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
      data: [
        { id: 1, name: 'warning 1', color: 'red', lineNo: 3 },
        { id: 2, name: 'warning 2', color: 'green', lineNo: 5 },
        { id: 3, name: 'warning 3', color: 'yellow', lineNo: 6 },
        { id: 4, name: 'warning 4', color: 'orange', lineNo: 10 },
      ],
      selected: [],
    };
  }

  /** 
   * Select / desect all the entry inside the table
   */
  handleSelectAllClick = (event, checked) => {
    if (checked) {
      this.setState({ selected: this.state.data.map(n => n.id) });
      return;
    }
    this.setState({ selected: [] });
  }

  /** 
   * Handle the click on the checkbox
   */
  handleSelectClick = (event, id) => {
    const { selected } = this.state;
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

    this.setState({ selected: newSelected });
  };

  /**
   * Check if the id is selected or not
   */
  isSelected = id => this.state.selected.indexOf(id) !== -1;

  render() {
    return (
      <Table>
        <TableHead>
          <TableRow>
            <TableCell checkbox>
              <Checkbox
                indeterminate={this.state.selected.length > 0 &&
                               this.state.selected.length < this.state.data.length}
                checked={this.state.selected.length === this.state.data.length}
                onChange={this.handleSelectAllClick}
              />
            </TableCell>
            <TableCell disablePadding>Warning Type</TableCell>
            <TableCell numeric>Color</TableCell>
          </TableRow>
        </TableHead>
        <TableBody>
          {this.state.data.map((n) => {
            const isSelected = this.isSelected(n.id);
            return (
              <TableRow
                role="checkbox"
                aria-checked={isSelected}
                tabIndex={-1}
                key={n.id}
                selected={isSelected}
                onClick={event => this.handleSelectClick(event, n.id)}
              >
                <TableCell checkbox>
                  <Checkbox checked={isSelected} />
                </TableCell>
                <TableCell disablePadding>{n.name}</TableCell>
                <TableCell numeric>
                  <Icon style={{ color: n.color }}>label</Icon>
                </TableCell>
              </TableRow>
            );
          })}
        </TableBody>
      </Table>
    );
  }
}

SummarySection.propTypes = {
  classes: PropTypes.object.isRequired,
};

export default withStyles(styles)(SummarySection);
