import React from 'react';
import PropTypes from 'prop-types';
import { withStyles } from 'material-ui/styles';
import AppBar from 'material-ui/AppBar';
import Toolbar from 'material-ui/Toolbar';
import Paper from 'material-ui/Paper';
import Grid from 'material-ui/Grid';
import Typography from 'material-ui/Typography';
import withRoot from '../components/withRoot';


const styles = theme => ({
  root: {
    width: '100%',
  },
  paper: {
    padding: 16,
    margin: 10,
    color: theme.palette.text.secondary,
  },
});

function MainStructure(props) {
  const classes = props.classes;

  return (
    <div className={classes.root}>
      <AppBar position="static" >
        <Toolbar>
          <Typography type="title">
            Dr_checker visualizer
          </Typography>
        </Toolbar>
      </AppBar>
      <Grid container spacing={24}>
        <Grid item xs={12}>
          <Paper className={classes.paper}>Hello World</Paper>
        </Grid>
      </Grid>
    </div>
  );
}

MainStructure.propTypes = {
  classes: PropTypes.object.isRequired, // eslint-disable-line react/forbid-prop-types
};

export default withRoot(withStyles(styles)(MainStructure));
