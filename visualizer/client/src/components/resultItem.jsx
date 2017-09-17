import React from 'react';
import PropTypes from 'prop-types';
import axios from 'axios';
import config from 'react-global-configuration';
import Card from 'material-ui/Card';
import { withStyles } from 'material-ui/styles';
import ResultItemHeader from './resultItemHeader.jsx';
import ResultItemBody from './resultItemBody.jsx';

const styles = theme => ({
  card: {
    color: theme.palette.common.fullWhite,
    backgroundColor: theme.palette.grey[700],
    marginTop: 10,
    marginBottom: 10,
  },
});

/**
 * This class encapsulate all the components relative to a single result
 * such as the summary of the warnings,the source code ofthe function etc...
 */
class ResultItem extends React.Component {
  constructor() {
    super();
    // at the begining we want all the cards to be NOT expanded
    this.state = {
      // indicates if the result card is expanded or not
      expanded: false,
      // lsit of warnings related to the analysis
      warnings: [],
    };
  }

  /**
   * Method that handle when the user clicks on the icon to expand the card
   * It basically toggle the expansion attribute everytime the user clicks on the button
   */
  handleExpandClick = () => {
    this.setState({ expanded: !this.state.expanded });
    this.fetchWarnings();
  }

  /**
   * Get the warnings related to this analysis 
   */
  fetchWarnings = () => {
    axios.get(`${config.get('endpoint')}/result/${this.props.functionName}`).then((response) => {
      if (response.data.success) {
        const parsedData = JSON.parse(response.data.data);
        this.setState({ warnings: parsedData });
      } else {
        // TODO : Display errors
      }
    });
  }

  render() {
    const classes = this.props.classes;
    return (
      <Card className={classes.card}>
        <ResultItemHeader
          title={this.props.functionName}
          expanded={this.state.expanded}
          handleExpandClick={this.handleExpandClick}
        />
        {/*
        <ResultItemBody
          expanded={this.state.expanded}
          filename={this.props.functionName}
        />
        */}
      </Card>
    );
  }
}

ResultItem.propTypes = {
  classes: PropTypes.object.isRequired,
  functionName: PropTypes.string.isRequired,
};

export default withStyles(styles)(ResultItem);
