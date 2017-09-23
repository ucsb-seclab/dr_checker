import React from 'react';
import PropTypes from 'prop-types';
import { withStyles } from 'material-ui/styles';
import AppBar from 'material-ui/AppBar';
import Toolbar from 'material-ui/Toolbar';
import Grid from 'material-ui/Grid';
import Typography from 'material-ui/Typography';
import withRoot from '../components/withRoot';
import ResultsList from '../components/resultsList.jsx';

const styles = {
  root: {
    width: '100%',
  },
};

/**
 * This class defines the main structure of te layout of the application
 * @param {object} prps: props inected by react framework
 */
function MainStructure(props) {
  const classes = props.classes;

  return (
    <div className={classes.root}>
      <AppBar position="static" >
        <Toolbar>
          <Typography type="title">
            Dr.Checker Warnings Visualizer
          </Typography>
        </Toolbar>
      </AppBar>
      <Grid item xs={12}>
        <ResultsList />
      </Grid>
    </div>
  );
}

MainStructure.propTypes = {
  classes: PropTypes.object.isRequired,
};

export default withRoot(withStyles(styles)(MainStructure));
