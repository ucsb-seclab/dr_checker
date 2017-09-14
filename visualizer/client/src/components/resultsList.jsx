import React from 'react';
import PropTypes from 'prop-types';
import Paper from 'material-ui/Paper';
import { withStyles } from 'material-ui/styles';
import ResultItem from './resultItem.jsx';

const styles = theme => ({
  paper: {
    padding: 16,
    margin: 10,
    color: theme.palette.text.secondary,
  },
});

/**
 * This class contains the list of results, returned from the server as 
 * its states and it renders one resultItem for each result
 */
class ResultsList extends React.PureComponent {
  constructor() {
    super();
    this.state = {
      value: null,
    };
  }

  render() {
    const classes = this.props.classes;
    return (
      <div>
        <Paper className={classes.paper}>
          <ResultItem functionName="JSON fun 1" />
          <ResultItem functionName="JSON fun 2" />
          <ResultItem functionName="JSON fun 3" />
        </Paper>
      </div>
    );
  }
}

ResultsList.propTypes = {
  classes: PropTypes.object.isRequired,
};

export default withStyles(styles)(ResultsList);
