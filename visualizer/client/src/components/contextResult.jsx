
import React from 'react';
import PropTypes from 'prop-types';
import Card from 'material-ui/Card';
import { withStyles } from 'material-ui/styles';
import Collapse from 'material-ui/transitions/Collapse';
import ContextItemHeader from './contextItemHeader.jsx';
import SourcefileResult from './sourcefileResult.jsx';

const styles = theme => ({
  card: {
    color: theme.palette.common.fullWhite,
    backgroundColor: theme.palette.grey[700],
    marginTop: 10,
    marginBottom: 10,
    boxShadow: 'none',
  },
  paddedCollapse: {
    marginTop: 20,
  },
});

/**
 * This class encapsulate all the components relative to a single context
 * such as the summary of the warnings,the source code ofthe function etc...
 */
class ContextResult extends React.Component {
  constructor() {
    super();
    // at the begining we want all the cards to be NOT expanded
    this.state = {
      // indicates if the result card is expanded or not
      expanded: false,
    };
  }

  /**
   * Method that handle when the user clicks on the icon to expand the card
   * It basically toggle the expansion attribute everytime the user clicks on the button
   */
  handleExpandClick = () => {
    this.setState({ expanded: !this.state.expanded });
  }

  render() {
    const classes = this.props.classes;
    return (
      <Card className={classes.card}>
        <ContextItemHeader
          title={this.props.contextTitle}
          expanded={this.state.expanded}
          handleExpandClick={this.handleExpandClick}
        />
        <Collapse in={this.state.expanded} transitionDuration="auto" unmountOnExit className={classes.paddedCollapse}>
          {
            this.props.alreadyFetched ?
              (Object.keys(this.props.warnings).map(key => (
                <SourcefileResult
                  key={key}
                  filename={key}
                  warnings={this.props.warnings[key]}
                />
              )))
              :
              (<h4>Loading...</h4>)
          }
        </Collapse>
      </Card>
    );
  }
}

ContextResult.propTypes = {
  classes: PropTypes.object.isRequired,
  contextTitle: PropTypes.string.isRequired,
  alreadyFetched: PropTypes.bool.isRequired,
  warnings: PropTypes.object.isRequired,
};

export default withStyles(styles)(ContextResult);
