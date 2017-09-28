import React from 'react';
import PropTypes from 'prop-types';
import Paper from 'material-ui/Paper';
import { withStyles } from 'material-ui/styles';
import axios from 'axios';
import config from 'react-global-configuration';
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
      // List of results returned by the server
      data: [],
    };
  }

  /**
   * Fetch resultsd list from server when the component is mounted
   */
  componentDidMount = () => {
    axios.get(`${config.get('endpoint')}/results`).then((response) => {
      if (response.data.success) {
        this.setState({ data: response.data.data });
      } else {
        // TODO : Display errors
      }
    });
  }

  render() {
    const classes = this.props.classes;
    return (
      <div>
        <Paper className={classes.paper}>
          {this.state.data.map((n, idx) => (<ResultItem key={idx} functionName={n} />))}
        </Paper>
      </div>
    );
  }
}

ResultsList.propTypes = {
  classes: PropTypes.object.isRequired,
};

export default withStyles(styles)(ResultsList);
